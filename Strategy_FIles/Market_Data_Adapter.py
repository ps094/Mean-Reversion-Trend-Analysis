import API_Wrapper
import Config
import BS
import os

from scipy.optimize import bisect
import time
import datetime
import numpy as np
import pandas as pd
pd.set_option('display.max_columns', None)
pd.set_option('display.max_rows', None)

import copy
from colorama import Fore, Back, Style
import warnings
warnings.filterwarnings("ignore")

from kiteconnect import KiteConnect
from pymongo import MongoClient
client = MongoClient(Config.DB_Hostname, Config.DB_Port)

def IVOL(C,S,T,K,opt_type):
    
    try:
        def Calc_Call(sig):
            return BS.bs_call(S,K,T/252,Config.interest_rate,sig)-C

        def Calc_Put(sig):
            return BS.bs_put(S,K,T/252,Config.interest_rate,sig)-C

        sigma = 0.5
        if opt_type=="CE":

            while BS.bs_call(S,K,T/252,Config.interest_rate,sigma)>C:
                sigma /= 2

            while BS.bs_call(S,K,T/252,Config.interest_rate,sigma)<C:
                sigma *= 2

            hi = sigma
            lo = hi/2
            return bisect(Calc_Call,lo,hi)    

        else:

            while BS.bs_put(S,K,T/252,Config.interest_rate,sigma)>C:
                sigma /= 2

            while BS.bs_put(S,K,T/252,Config.interest_rate,sigma)<C:
                sigma *= 2

            hi = sigma
            lo = hi/2
            try:
                return bisect(Calc_Put,lo,hi)
            except:
                return np.nan
    
    except Exception as e:
        return np.nan

class Market_Data_Adapter:
    
    def __init__(self, parameters):
        
#        parameters = {"date_today": "2023-09-17", 
#                      'underlying': ['NSE:NIFTY 50', 'NSE:NIFTY BANK'],
#                      'symbol_map' : {"NSE:NIFTY 50" : "NIFTY", "NSE:NIFTY BANK" : "BANKNIFTY"},
#                      'reversed_symbol_map' : {"NIFTY" : "NSE:NIFTY 50", "BANKNIFTY" : "NSE:NIFTY BANK"},
#                      'num_instruments' : {"NIFTY": 100, "BANKNIFTY": 100}}
        
        #Cleaning DataBase
        print(Fore.GREEN, client.Live_Trading.Index_Tracker.delete_many({}).deleted_count, f" Documents Deleted From Index_Tracker")
        print(Fore.GREEN, client.Live_Trading.Instrument_Universe.delete_many({}).deleted_count, f" Documents Deleted From Instrument_Universe")
        print(Fore.GREEN, client.Live_Trading.Price_Packet.delete_many({}).deleted_count, f" Documents Deleted From Price_Packet")
        print(Fore.GREEN, client.Live_Trading.Vol_Tracker.delete_many({}).deleted_count, f" Documents Deleted From Vol_Tracker")
        print(Style.RESET_ALL)
        #underlying = ['NSE:NIFTY 50', 'NSE:NIFTY BANK']
        self.parameters = copy.deepcopy(parameters)
        
        #initialize APIs
        self.API_Interface = API_Wrapper.API_Wrapper(market_adapter = True, paper_trading = True)

        #declare index price dictionary 
        self.previous_prices = {}
        self.current_prices = {}
        
        #declare focus / instrument list
        self.focus_instruments =[]
        self.instrument_list = None

        #initialize prices and instrument_lists
        self._get_prices()
        self._get_instrument_list()

        #initialize days to expiry
        self.days_to_expiry = {}
        self._get_days_to_expiry()
        
        #get backup_vol
        vol_surface = pd.DataFrame(client[f'{Config.Data_DB}']['Vol_Surface'].find())
        self.backup_vol = vol_surface[vol_surface.date == sorted(vol_surface.date, reverse=True)[0]].set_index('SYMBOL').vol_0.to_dict()
        
        #declaring vol_tracker and index_tracker and helper maps
        self.current_instrument_price_map = {}
        self.previous_instrument_price_map = {}
        self.seq_no = 0
        self.vol_tracker = None #pd.DataFrame columns -> tradingsymbol, vol, seq_no
        self.previous_vol_tracker = None #pd.DataFrame columns -> tradingsymbol, vol, seq_no

        self.index_tracker = None #pd.DataFrame([{'underlying':, 'price':, 'timestamp':}])
        self.previous_index_tracker = None
        
        self._get_prices()
        self._update_vol_tracker(self.days_to_expiry)
        
        print(Fore.GREEN, "Market Adapter Initialized")
        print(Style.RESET_ALL)
        
    def _get_prices(self):
        
        #fetching market prices
        try:
            
            quotes = self.API_Interface.Quote(self.parameters['underlying'] + self.focus_instruments)
            for instrument in self.parameters['underlying'] + self.focus_instruments:

                if instrument in quotes:

                    if instrument in self.parameters['underlying']:

                        try:    
                            quotes[instrument]['last_price'] /= 1
                            if quotes[instrument]['last_price'] == 0:
                                raise RuntimeError("Zero Price Error")
                            self.current_prices[self.parameters['symbol_map'][instrument]] = quotes[instrument]['last_price']

                        except Exception as e:
                            print(Fore.YELLOW, f"{e}, Invalid price for {instrument}")
                            if self.parameters['symbol_map'][instrument] in self.previous_prices:
                                self.current_prices[self.parameters['symbol_map'][instrument]] = self.previous_prices[self.parameters['symbol_map'][instrument]]
                            else:
                                tries = 5
                                while tries > 0:
                                    try:
                                        sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
                                        self.current_prices[self.parameters['symbol_map'][instrument]] = float(input(Back.LIGHTCYAN_EX + Fore.RED + f"Enter price for {instrument}"))
                                        print(Style.RESET_ALL)
                                        break
                                    except Exception as e:
                                        tries -= 1
                                        if tries == 0:
                                            raise RuntimeError(Fore.RED + f"{e}, Invalid Price;")
                                        else:
                                            print(Fore.YELLOW, f"{e}, Error!! Try Again (tries_left: {tries})")
                                            print(Style.RESET_ALL)
                                            
                    else:
                        self.current_prices[self.parameters['symbol_map'][instrument]] = quotes[instrument]

                else:
                    print(Fore.YELLOW + f"{instrument} quote unavailable")
                    if self.parameters['symbol_map'][instrument] in self.previous_prices:
                        self.current_prices[self.parameters['symbol_map'][instrument]] = self.previous_prices[self.parameters['symbol_map'][instrument]]
                    elif instrument in self.parameters['underlying']:
                        tries = 5
                        while tries > 0:
                            try:
                                sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
                                self.current_prices[self.parameters['symbol_map'][instrument]] = float(input(Back.LIGHTCYAN_EX + Fore.RED + f"Enter price for {instrument}"))                                
                                print(Style.RESET_ALL)
                                break
                            except Exception as e:
                                tries -= 1
                                if tries == 0:
                                    raise RuntimeError(Fore.RED + f"{e}, Invalid Price;")
                                else:
                                    print(Fore.YELLOW, f"{e}, Error!! Try Again (tries_left: {tries})")
                                    print(Style.RESET_ALL)
                                    
                    else:
                        self.current_prices[self.parameters['symbol_map'][instrument]] = None
            
            self.index_tracker = []
            timestamp = datetime.datetime.now().timestamp()
            for underlying in self.parameters["underlying"]:
                self.index_tracker.append({'underlying' : self.parameters['symbol_map'][underlying],
                                           'price': self.current_prices[self.parameters['symbol_map'][underlying]],
                                           'timestamp' : timestamp})
            self.index_tracker = pd.DataFrame(self.index_tracker)
            
            #creating backup
            self.previous_prices = copy.deepcopy(self.current_prices)
            self.previous_index_tracker = copy.deepcopy(self.index_tracker)
            
        except Exception as e:
            self.current_prices = copy.deepcopy(self.previous_prices)
            self.index_tracker = copy.deepcopy(self.previous_index_tracker)
            print(Fore.RED, f"{e}, Copying previous prices")
            print(Style.RESET_ALL)
            
        #inserting into DB
        tries = 5
        while tries > 0:
            try:
                client.Live_Trading.Index_Tracker.insert_many(self.index_tracker.to_dict('records'))
                break
            except Exception as e:
                tries -= 1
                
                if tries == 0:
                    print(Fore.RED, f"{e}, Could Not Push Index Levels To DB, Failed")
                    print(Style.RESET_ALL)
                    break
                    
                else:
                    print(Fore.YELLOW, f"{e}, Could Not Push Index Levels To DB, Trying Again")
                    print(Style.RESET_ALL)
                    time.sleep(1)
            
        #saving price packet
        self.process_price_packet_and_insert_into_db()
        
    def process_price_packet_and_insert_into_db(self):

        try:
            underlying_list = []
            for underlying in self.parameters["underlying"]:
                underlying_list.append(self.parameters["symbol_map"][underlying])

            price_packet_dataframe = []
            for instrument in self.current_prices:

                if instrument in underlying_list:
                    price_packet_dataframe.append({"instrument" : instrument, "last_price" : self.current_prices[instrument],
                                                   "buy_price_1" : self.current_prices[instrument], "buy_quantity_1" : 0,
                                                   "buy_price_2" : self.current_prices[instrument], "buy_quantity_2" : 0,
                                                   "buy_price_3" : self.current_prices[instrument], "buy_quantity_3" : 0,
                                                   "buy_price_4" : self.current_prices[instrument], "buy_quantity_4" : 0,
                                                   "buy_price_5" : self.current_prices[instrument], "buy_quantity_5" : 0,
                                                   "sell_price_1" : self.current_prices[instrument], "sell_quantity_1" : 0,
                                                   "sell_price_2" : self.current_prices[instrument], "sell_quantity_2" : 0,
                                                   "sell_price_3" : self.current_prices[instrument], "sell_quantity_3" : 0,
                                                   "sell_price_4" : self.current_prices[instrument], "sell_quantity_4" : 0,
                                                   "sell_price_5" : self.current_prices[instrument], "sell_quantity_5" : 0})

                else:
                    price_packet_dataframe.append({"instrument" : instrument, "last_price" : self.current_prices[instrument]['last_price'],
                                                   "buy_price_1" : self.current_prices[instrument]['depth']['buy'][0]['price'], "buy_quantity_1" : self.current_prices[instrument]['depth']['buy'][0]['quantity'],
                                                   "buy_price_2" : self.current_prices[instrument]['depth']['buy'][1]['price'], "buy_quantity_2" : self.current_prices[instrument]['depth']['buy'][1]['quantity'],
                                                   "buy_price_3" : self.current_prices[instrument]['depth']['buy'][2]['price'], "buy_quantity_3" : self.current_prices[instrument]['depth']['buy'][2]['quantity'],
                                                   "buy_price_4" : self.current_prices[instrument]['depth']['buy'][3]['price'], "buy_quantity_4" : self.current_prices[instrument]['depth']['buy'][3]['quantity'],
                                                   "buy_price_5" : self.current_prices[instrument]['depth']['buy'][4]['price'], "buy_quantity_5" : self.current_prices[instrument]['depth']['buy'][4]['quantity'],
                                                   "sell_price_1" : self.current_prices[instrument]['depth']['sell'][0]['price'], "sell_quantity_1" : self.current_prices[instrument]['depth']['sell'][0]['quantity'],
                                                   "sell_price_2" : self.current_prices[instrument]['depth']['sell'][1]['price'], "sell_quantity_2" : self.current_prices[instrument]['depth']['sell'][1]['quantity'],
                                                   "sell_price_3" : self.current_prices[instrument]['depth']['sell'][2]['price'], "sell_quantity_3" : self.current_prices[instrument]['depth']['sell'][2]['quantity'],
                                                   "sell_price_4" : self.current_prices[instrument]['depth']['sell'][3]['price'], "sell_quantity_4" : self.current_prices[instrument]['depth']['sell'][3]['quantity'],
                                                   "sell_price_5" : self.current_prices[instrument]['depth']['sell'][4]['price'], "sell_quantity_5" : self.current_prices[instrument]['depth']['sell'][4]['quantity']})

            price_packet_dataframe = pd.DataFrame(price_packet_dataframe).set_index('instrument').astype(float).fillna(0).reset_index().to_dict('records')

        except Exception as e:
            print(Fore.RED, f"{e}, Unable To Convert Price Packet To DataFrame")
            print(Style.RESET_ALL)
            price_packet_dataframe = pd.DataFrame()

        if len(price_packet_dataframe) != 0:

            tries = 5
            while tries != 0:
                try:
                    client.Live_Trading.Price_Packet.delete_many({})
                    break
                except Exception as e:
                    tries -= 1
                    if tries == 0:
                        tries -= 1
                        print(Fore.RED, f"{e}, Unable To Delete Previous Price Packet")
                        print(Style.RESET_ALL)
                        break
                    else:
                        print(Fore.YELLOW, f"{e}, Could Not Delete Previous Price Packet From DB, Trying Again (tries_left: {tries})")
                        print(Style.RESET_ALL)
                        time.sleep(1)

            if tries >=0:

                tries = 5
                while tries != 0:
                    try:
                        client.Live_Trading.Price_Packet.insert_many(price_packet_dataframe)
                        break
                    except Exception as e:
                        tries -= 1
                        if tries == 0:
                            tries -= 1
                            print(Fore.RED, f"{e}, Unable To Insert New Price Packet")
                            print(Style.RESET_ALL)
                            break
                        else:
                            print(Fore.YELLOW, f"{e}, Could Not Insert New Price Packet Into DB, Trying Again (tries_left: {tries})")
                            print(Style.RESET_ALL)
                            time.sleep(1)

    def _get_instrument_list(self):
        
        if self.instrument_list is None:
            
            print(Fore.GREEN, f"Filtering Instruments Using These Underlying Prices {self.current_prices}")
            print(Style.RESET_ALL)
            sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
            user_input = input(Back.LIGHTCYAN_EX + Fore.BLACK + "All Okay? (Enter Any Character If Otherwise)")
            print(Style.RESET_ALL)
            
            if len(user_input) != 0:
                
                for instrument, price in self.current_prices.items():
                    
                    tries = 5
                    while tries > 0:
                        try:
                            sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
                            user_input = input(Back.LIGHTCYAN_EX + Fore.BLACK + f"Enter Price For {instrument}: ")
                            print(Style.RESET_ALL)
                            self.current_prices[instrument] = float(user_input.strip())
                            break
                        except Exception as e:
                            tries -= 1
                            if tries == 0:
                                raise RuntimeError(Fore.RED + f"{e} Bad Price")
                            else:
                                print(Fore.RED + f"{e}, Error Try Again (tries_left: {tries})")
                                print(Style.RESET_ALL)
                                
                self.previous_prices = copy.deepcopy(self.current_prices)

        self.instrument_list = self.API_Interface.Instruments()
        self.instrument_list = self.instrument_list[self.instrument_list.underlying.isin(self.parameters['reversed_symbol_map'])]
        processed_instrument_list = None

        for underlying in self.parameters['reversed_symbol_map']:
            
            instrument_list = self.instrument_list[self.instrument_list.underlying == underlying]
            futures = instrument_list[instrument_list.instrument_type == 'FUT']
            futures = futures[futures.expiry.isin(sorted(futures.expiry.unique())[:2])]
            options = instrument_list[instrument_list.instrument_type.isin(['CE', 'PE'])]
            options = options[options.expiry.isin(sorted(options.expiry.unique())[:2])]

            options["strike_diff"] = abs(options.strike - self.current_prices[underlying])
            options.sort_values('strike_diff', inplace = True)
            options = options.head(self.parameters['num_instruments'][underlying])

            if processed_instrument_list is None:
                processed_instrument_list = pd.concat([options, futures])
            else:
                processed_instrument_list = pd.concat([processed_instrument_list, options, futures])

        self.instrument_list = copy.deepcopy(processed_instrument_list.drop(columns = ['strike_diff']))
        if len(self.instrument_list) < 100:
            raise RuntimeError(Fore.RED, f"Not enough Instruments, Got Only {len(self.instrument_list)}")
        
        else:            
            for tradingsymbol in self.instrument_list.tradingsymbol:

                modified_tradingsymbol = "NFO:" + tradingsymbol
                self.focus_instruments.append(modified_tradingsymbol)
                self.parameters['symbol_map'][modified_tradingsymbol] = tradingsymbol
                self.parameters['reversed_symbol_map'][tradingsymbol] = modified_tradingsymbol
        
        #entering into DB
        print(Fore.GREEN, len(client.Live_Trading.Instrument_Universe.insert_many(self.instrument_list.to_dict('records')).inserted_ids),f" Documents Entered In Instrument_Universe")

    def _get_days_to_expiry(self):
        
        days_to_expiry = pd.DataFrame(client.Strategy.Days_To_Expiry.find({"date" : self.parameters['date_today']})).drop(columns=['_id','date']).to_dict('records')
        days_to_expiry = {temp['underlying']: {'current_week': temp['current_week'], 'next_week': temp['next_week']} for temp in days_to_expiry}

        for underlying in self.parameters["underlying"]:

            opt_expiry = sorted(self.instrument_list[(self.instrument_list.underlying == self.parameters["symbol_map"][underlying])&(self.instrument_list.instrument_type == "CE")].expiry.unique())
            
            if len(opt_expiry) < 2:
                print(Fore.RED, f"Couldnt Find Two Consequtive Expiries For {underlying}")
                print(Style.RESET_ALL)
                continue
                
            self.days_to_expiry[opt_expiry[0]] = days_to_expiry[self.parameters["symbol_map"][underlying]]["current_week"]
            self.days_to_expiry[opt_expiry[1]] = days_to_expiry[self.parameters["symbol_map"][underlying]]["next_week"]
        
    def _update_vol_tracker(self, time_to_expiry):
        
        try:
            #calculating mid price or ltp for each option instrument
            for tradingsymbol in self.instrument_list[self.instrument_list.instrument_type != "FUT"].tradingsymbol:

                try:
                    last_price = self.current_prices[tradingsymbol]['last_price']
                    last_price /= 1
                    if last_price == 0:
                        raise RuntimeError()
                except Exception as e:
                    last_price = None

                try:
                    buy = self.current_prices[tradingsymbol]['depth']['buy'][0]['price']
                    buy /= 1
                    if buy == 0:
                        raise RuntimeError()
                except Exception as e:
                    buy = None

                try:
                    sell = self.current_prices[tradingsymbol]['depth']['sell'][0]['price']
                    sell /= 1
                    if sell == 0:
                        raise RuntimeError()
                except Exception as e:
                    sell = None

                mid = np.nan
                if buy is None or sell is None:
                    if last_price is None:
                        if tradingsymbol in self.previous_instrument_price_map:
                            mid = self.previous_instrument_price_map[tradingsymbol]
                    else:
                        mid = last_price
                else:
                    mid = 0.5 * (buy + sell)

                self.current_instrument_price_map[tradingsymbol] = mid

            #ivol calculation
            options = self.instrument_list[self.instrument_list.instrument_type != "FUT"]
            options["underlying_price"] = options.underlying.map(self.current_prices)
            options["time_to_expiry"] = options.expiry.map(time_to_expiry)
            options["option_price"] = options.tradingsymbol.map(self.current_instrument_price_map)
            options['vol'] = options[['option_price', 'underlying_price', 'time_to_expiry', 'strike', 'instrument_type']].dropna().apply(
                    lambda x : IVOL(x.option_price, x.underlying_price, x.time_to_expiry, x.strike, x.instrument_type),axis=1)

            # filling missing vols with neighbouring strikes
            tradingsymbol_underlying_map = options.set_index('tradingsymbol').underlying.to_dict()
            tradingsymbol_expiry_map = options.set_index('tradingsymbol').underlying.to_dict()
            options = options.sort_values(['underlying', 'expiry', 'strike', 'instrument_type']).groupby(['underlying', 'expiry']).ffill().bfill()
            options['underlying'] = options.tradingsymbol.map(tradingsymbol_underlying_map)
            options['expiry'] = options.tradingsymbol.map(tradingsymbol_expiry_map)
            options.vol = np.where(options.vol.isna(), options.underlying.map(self.backup_vol), options.vol)

            # recalculating fair prices
            call_options = options[options.instrument_type == "CE"]
            put_options = options[options.instrument_type == "PE"]
            call_options["vol_price"] = call_options.apply(lambda x : BS.bs_call(x.underlying_price, x.strike, x.time_to_expiry/252, Config.interest_rate, x.vol), axis = 1)
            put_options["vol_price"] = put_options.apply(lambda x : BS.bs_put(x.underlying_price, x.strike, x.time_to_expiry/252, Config.interest_rate, x.vol), axis = 1)
            options = pd.concat([call_options, put_options]).sort_values(['underlying', 'expiry', 'strike', 'instrument_type'])

            #saving work
            self.vol_tracker = options[['tradingsymbol', 'vol']]
            self.current_instrument_price_map = options.set_index('tradingsymbol').vol_price.to_dict()
            
            #backing up
            self.previous_instrument_price_map = copy.deepcopy(self.current_instrument_price_map)
            self.previous_vol_tracker = copy.deepcopy(self.vol_tracker)
            
        except Exception as e:
            print(Fore.RED, f"{e}, Could Not Calculate Current Vols, Uploding Previous Vols")
            print(Style.RESET_ALL)
            self.vol_tracker = copy.deepcopy(self.previous_vol_tracker)
            self.current_instrument_price_map = copy.deepcopy(self.previous_instrument_price_map)
        
        #inserting into DB
        self.seq_no += 1
        tries = 5
        while tries > 0:
            try:
                self.vol_tracker["seq_no"] = self.seq_no
                client.Live_Trading.Vol_Tracker.insert_many(self.vol_tracker.to_dict('records'))
                break
                
            except Exception as e:
                tries -= 1
                if tries == 0:
                    print(Fore.YELLOW, f"{e}, Could Not Push Vols To DB, Trying Again (tries_left: {tries})")
                    print(Style.RESET_ALL)
                    self.seq_no -= 1
                    break
                    
                else:
                    print(Fore.RED, f"{e}, Could Not Push Vols To DB, Failed")
                    print(Style.RESET_ALL)
                    time.sleep(1)
    
    def Start(self):
        
        print(Fore.GREEN, "\nMarket Adapter Live")
        print(Style.RESET_ALL)
        days_since_start = (datetime.datetime.now().timestamp() - datetime.datetime.strptime(f"{self.parameters['date_today']}" + " 09:15:00", "%Y-%m-%d %H:%M:%S").timestamp()) / 22500
        previous_vol_upload = -0.25
        
        try:
            index_tracker_count = len(pd.DataFrame(client.Live_Trading.Index_Tracker.find()))
        except Exception as e:
            print(f"{e}, Could Not Retrieve Index Tracker Count")
            index_tracker_count = 0
            
        try:
            vol_tracker_count = len(pd.DataFrame(client.Live_Trading.Vol_Tracker.find()))
        except Exception as e:
            print(f"{e}, Could Not Retrieve Vol Tracker Count")
            vol_tracker_count = 0
            
        try:
            price_packet_count = len(pd.DataFrame(client.Live_Trading.Price_Packet.find()))
        except Exception as e:
            print(f"{e}, Could Not Retrieve Price Packet Count")
            price_packet_count = 0
        
        while days_since_start <=1:
            
            #index and price packet
            self._get_prices()
            
            if days_since_start * 375 - previous_vol_upload >=0.5:
                
                #vol tracker
                time_to_expiry = {expiry_date: days_to_expiry - days_since_start for expiry_date, days_to_expiry in self.days_to_expiry.items() }
                self._update_vol_tracker(time_to_expiry)
                previous_vol_upload += 0.5
                
                #checking of data adapter is live and wroking
                print(Fore.BLUE, f"\nMarket Adapter Status at {datetime.datetime.now()}")
                print(Style.RESET_ALL)
                
                #Index Tracker
                try:
                    index_tracker_count_temp = len(pd.DataFrame(client.Live_Trading.Index_Tracker.find()))
                except Exception as e:
                    print(f"{e}, Could Not Retrieve Index Tracker Count")
                    index_tracker_count_temp = 0

                if index_tracker_count_temp > index_tracker_count:
                    index_tracker_count = index_tracker_count_temp
                    print(Fore.GREEN, "Index Tracker Is Live ")
                    print(Style.RESET_ALL)
                    
                else:
                    print(Fore.RED, "Index Tracker Is Stale")
                    print(Style.RESET_ALL)
                    
                    
                #vol tracker
                try:
                    vol_tracker_count_temp = len(pd.DataFrame(client.Live_Trading.Vol_Tracker.find()))
                except Exception as e:
                    print(f"{e}, Could Not Retrieve Vol Tracker Count")
                    vol_tracker_count_temp = 0

                if vol_tracker_count_temp > vol_tracker_count:
                    vol_tracker_count = vol_tracker_count_temp
                    print(Fore.GREEN, "Vol Tracker Is Live")
                    print(Style.RESET_ALL)   
                    
                else:
                    print(Fore.RED, "Vol Tracker Is Stale")
                    print(Style.RESET_ALL)

                    
                #price packet    
                try:
                    price_packet_count_temp = len(pd.DataFrame(client.Live_Trading.Price_Packet.find()))
                except Exception as e:
                    price_packet_count_temp = 0
                    price_packet_count = price_packet_count_temp
                    print(f"{e}, Could Not Retrieve Price Packet Count")
                
                if price_packet_count_temp == len(self.current_prices):
                    print(Fore.GREEN, "Price Packet Is Live")
                    print(Style.RESET_ALL)                    
                else:
                    print(Fore.RED, "Price Packet Is Incomplete / Empty")
                    print(Style.RESET_ALL)
                                
            days_since_start = (datetime.datetime.now().timestamp() - datetime.datetime.strptime(f"{self.parameters['date_today']}" + " 09:15:00", "%Y-%m-%d %H:%M:%S").timestamp()) / 22500

sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
print(Back.LIGHTCYAN_EX, Fore.BLACK)
parameters = {"date_today": input("Enter Today's Date? "), 
              'underlying': ['NSE:NIFTY 50'], 
              'symbol_map' : {"NSE:NIFTY 50" : "NIFTY"}, 
              'reversed_symbol_map' : {"NIFTY" : "NSE:NIFTY 50"}, 
              'num_instruments' : {"NIFTY": 200}}
"""
parameters = {"date_today": input("Enter Today's Date? "), 
              'underlying': ['NSE:NIFTY 50', 'NSE:NIFTY BANK'], 
              'symbol_map' : {"NSE:NIFTY 50" : "NIFTY", "NSE:NIFTY BANK" : "BANKNIFTY"}, 
              'reversed_symbol_map' : {"NIFTY" : "NSE:NIFTY 50", "BANKNIFTY" : "NSE:NIFTY BANK"}, 
              'num_instruments' : {"NIFTY": 200, "BANKNIFTY": 200}}
"""
print(Style.RESET_ALL)

print(Fore.LIGHTMAGENTA_EX, Back.BLACK, f"\nparameters")
for key, value in parameters.items():
    print(f"\n{key} : \n{value}, \ntype : {type(value)}")
print(Style.RESET_ALL)

self = Market_Data_Adapter(parameters)
self.Start()