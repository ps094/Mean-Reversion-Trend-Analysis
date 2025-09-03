import Directional_Signal_Strategizer
import Config
import BS

import os
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

from pymongo import MongoClient
client = MongoClient(Config.DB_Hostname, Config.DB_Port)

class Strategy_Central_Command:
    
    def __init__(self, parameters):

        #cleaning DataBase
        print(client.Live_Trading.Order_Requests.delete_many({}).deleted_count, " Orders Cleared From Order Requests DB")
        print(client.Live_Trading.Order_Acks.delete_many({}).deleted_count, " Orders Cleared From Order Acks DB")
        print(client.Live_Trading.Momentum_Indicator.delete_many({}).deleted_count, " Indicators Cleared From Momentum Indicator DB")
        
        #parameters = {date_today : '2023-06-30', date_yesterday : '2023-06-28', underlying : ['NIFTY', 'BANKNIFTY'], capital : 500000}
        self.date_today = parameters["date_today"]
        self.date_yesterday = parameters["date_yesterday"]
        self.underlying = parameters["underlying"]
        self.capital = parameters["capital"]
        self.momentum_threshold = parameters["momentum_threshold"]
        self.momentum_lags = parameters["momentum_lags"]
        self.momentum_switch = parameters["momentum_switch"]
        
        self.order_req_number = 0
        self.order_ack_number = 0
        self.current_batch_id = 0
        self.pnl = {}
        self.momentum_indicator = {underlying : 0 for underlying in self.underlying}
        
        #initialize underlying close from previous date
        self.previous_close = {}
        self._get_previous_close()
        self.current_close = copy.deepcopy(self.previous_close)
        
        #initialize strategy templates and fetch tradeable strategies
        self.tradeable_strategies = None
        self.strategy_variants = None
        self._get_tradeable_strategies()
        
        #initialize instrument list
        self.instrument_list = None
        self._get_instrument_list()

        #initialize days to expiry
        self.days_to_expiry = {}
        self._get_days_to_expiry()
        
        #initialize strategizers
        self.strategizers = {}
        self._init_strategizers()
        
        print(Fore.GREEN + "Strategy Command Initialized")
        
    def _get_previous_close(self):
        
        try:
            
            for underlying in self.underlying:
                df = pd.DataFrame(client.Strategy[f"{underlying}OHLC"].find({"date" : self.date_yesterday}))
                self.previous_close[underlying] = float(df.sort_values('batch_id').close.tolist()[-1])

        except Exception as e:
            raise RuntimeError(Fore.RED + "Previous Close Missing")
            
    def _get_tradeable_strategies(self):
        
        self.strategy_variants = pd.DataFrame(client.Strategy.Strategy_Variants.find()).drop(columns=['_id','underlying']).sort_values('strategy_variant').to_dict('records')
        
        #strategy_variant, #pos_param, #mov_param, #prof_param, #date, #underlying, #rtd
        self.tradeable_strategies = pd.DataFrame(client.Live_Trading.Live_Strategies.find({'date':self.date_today}))
                
        if len(self.tradeable_strategies)==0:
            raise RuntimeError(Fore.RED + "No Tradeable Strategies")
            
        else:

            self.tradeable_strategies.strategy_variant = self.tradeable_strategies.strategy_variant.astype(int)
            self.tradeable_strategies.pos_param = self.tradeable_strategies.pos_param.astype(float)
            self.tradeable_strategies.mov_param = self.tradeable_strategies.mov_param.astype(float)
            self.tradeable_strategies.prof_param = self.tradeable_strategies.prof_param.astype(float)
            self.tradeable_strategies.date = self.tradeable_strategies.date.astype(str)
            self.tradeable_strategies.underlying = self.tradeable_strategies.underlying.astype(str)
            self.tradeable_strategies.rtd = self.tradeable_strategies.rtd.astype(float)
            self.tradeable_strategies = self.tradeable_strategies.drop(columns=['_id']).to_dict('records')
            
    def _get_instrument_list(self):
        
        self.instrument_list = pd.DataFrame(client.Live_Trading.Instrument_Universe.find())
        if len(self.instrument_list)==0 or len(set(self.underlying + self.instrument_list.underlying.tolist())) != len(self.underlying):
            raise RuntimeError(Fore.RED + "Empty / Improper Instrument List")
        
        self.instrument_list.drop(columns=['_id'], inplace = True)
        self.instrument_list.instrument_token = self.instrument_list.instrument_token.astype(int)
        self.instrument_list.exchange_token = self.instrument_list.exchange_token.astype(int)
        self.instrument_list.tradingsymbol = self.instrument_list.tradingsymbol.astype(str)
        self.instrument_list.underlying = self.instrument_list.underlying.astype(str)
        self.instrument_list.expiry = self.instrument_list.expiry.astype(str)
        self.instrument_list.strike = self.instrument_list.strike.astype(float)
        self.instrument_list.tick_size = self.instrument_list.tick_size.astype(float)
        self.instrument_list.lot_size = self.instrument_list.lot_size.astype(int)
        self.instrument_list.instrument_type = self.instrument_list.instrument_type.astype(str)
        self.instrument_list.exchange = self.instrument_list.exchange.astype(str)

    def _get_days_to_expiry(self):
        
        days_to_expiry = pd.DataFrame(client.Strategy.Days_To_Expiry.find({"date" : self.date_today})).drop(columns=['_id','date']).to_dict('records')
        days_to_expiry = {temp['underlying']: {'current_week': temp['current_week'], 'next_week': temp['next_week']} for temp in days_to_expiry}

        for underlying in self.underlying:

            opt_expiry = sorted(self.instrument_list[(self.instrument_list.underlying == underlying) & (self.instrument_list.instrument_type == "CE")].expiry.unique())

            if len(opt_expiry) < 2:
                print(Fore.RED, f"Couldnt Find Two Consequtive Expiries For {underlying}")
                print(Style.RESET_ALL)
                continue
                
            self.days_to_expiry[opt_expiry[0]] = days_to_expiry[underlying]["current_week"]
            self.days_to_expiry[opt_expiry[1]] = days_to_expiry[underlying]["next_week"]

    def _init_strategizers(self):

        for strategy in self.tradeable_strategies:
            variant = self.strategy_variants[strategy["strategy_variant"]-1]
            
            tries = 5
            while tries > 0:
            
                try:
                    
                    print(Back.LIGHTCYAN_EX, Fore.BLACK)
                    strategy_params = {"strategy_type": variant["strategy_type_parameter"], 
                                       "strategy_version": variant["strategy_version_parameter"],
                                       "previous_close": self.previous_close[strategy["underlying"]],
                                       "position_parameter": variant["position_multiplier"] * strategy["pos_param"],
                                       "extreme_move_parameter": variant["extreme_move_multiplier"] * strategy["mov_param"], 
                                       "abstinence_parameter": variant["abstinence_multiplier"] * strategy["pos_param"], 
                                       "straight_abstinence_parameter": variant["straight_abstinence_multiplier"] * strategy["pos_param"],
                                       "straight_reversal_parameter": variant["straight_reversal_multiplier"] * strategy["pos_param"],
                                       "directional_move_parameter": variant["directional_move_multiplier"] * strategy["mov_param"], 
                                       "initial_move_parameter": variant["initial_move_multiplier"] * strategy["mov_param"],
                                       "initial_time_parameter": 10, 
                                       "take_profit_parameter": variant["take_profit_multiplier"] * strategy["prof_param"], 
                                       "hedge_manage_parameter": variant["hedge_manage_multiplier"] * strategy["prof_param"], 
                                       "stop_loss_parameter": variant["stop_loss_multiplier"] * strategy["prof_param"], 
                                       "current_week": variant["week_parameter"], 
                                       "position_ratio": variant["position_ratio"]}
                    
                    sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
                    parameters = {"underlying": strategy["underlying"], 
                                  "date": self.date_today,
                                  "strategy_variant": variant["strategy_variant"],
                                  "num_legs": int(input(f"Strategy_Template : {variant} \n\nStrategy_Parameters : {strategy} \n\nEnter Number Of Legs For This Strategy : ")),
                                  "strategy_params": strategy_params,
                                  "instrument_list": self.instrument_list,
                                  "strike_multiples": int(input("Enter Strike Multiple For This Strategy : ")), 
                                  "days_to_expiry": self.days_to_expiry,
                                  "allocated_capital": self.capital/len(self.tradeable_strategies)}
                    
                    print(Style.RESET_ALL)
                    break
                    
                except Exception as e:
                    
                    tries -= 1
                    if tries == 0:
                        raise RuntimeError(Fore.RED + f"{e}, {strategy} Not Initialized;\n")
                    else:
                        print(Fore.RED, f"{e}, Error!! Try Again\n")
                        print(Style.RESET_ALL)
                        
            strategy_tag = str(variant["strategy_variant"]) + strategy["underlying"]
            self.strategizers[f"{strategy_tag}"] = Directional_Signal_Strategizer.Directional_Signal_Strategizer(parameters)
            self.pnl[strategy_tag] = {"realized_pnl" : 0, "un-realized_pnl" : 0}
            
    def _get_price_guide(self):

        try:
            tries = 5
            while tries > 0:
                try:    
                    vols = pd.DataFrame(client.Live_Trading.Vol_Tracker.find())
                    break
                except Exception as e:
                    tries -= 1
                    if tries == 0:
                        print(Fore.RED, f"{e}, Could Not Retrieve Vols From DB, Failed")
                        print(Style.RESET_ALL)
                        vols = pd.DataFrame()
                        break
                        
                    else:
                        print(Fore.RED, f"{e}, Could Not Retrieve Vols From DB, Trying Again (tries_left: {tries})")
                        print(Style.RESET_ALL)
                        time.sleep(1)
                        
            if len(vols) != 0:
                
                vols.drop(columns=['_id'], inplace = True)
                vols = vols[vols.seq_no > vols.seq_no.max() - 10]
                vols = vols.groupby('tradingsymbol').mean()[['vol']].reset_index()
                vols["strike"]= vols.tradingsymbol.map(self.instrument_list.set_index('tradingsymbol').strike.to_dict())
                vols["underlying"]= vols.tradingsymbol.map(self.instrument_list.set_index('tradingsymbol').underlying.to_dict())
                vols["instrument_type"]= vols.tradingsymbol.map(self.instrument_list.set_index('tradingsymbol').instrument_type.to_dict())
                vols["expiry"]= vols.tradingsymbol.map(self.instrument_list.set_index('tradingsymbol').expiry.to_dict())

                days_since_start = (datetime.datetime.now().timestamp() - datetime.datetime.strptime(f"{self.date_today}" + " 09:15:00", "%Y-%m-%d %H:%M:%S").timestamp()) / 22500 
                days_since_start = max(0, min(days_since_start, 0.96))
                vols_calls = vols[vols.instrument_type=="CE"]
                vols_puts = vols[vols.instrument_type=="PE"]
                vols_calls["price"] = vols_calls.apply(lambda x : BS.bs_call(self.current_close[x.underlying], x.strike, (self.days_to_expiry[x.expiry] - days_since_start)/252, Config.interest_rate, x.vol), axis = 1)
                vols_puts["price"] = vols_puts.apply(lambda x : BS.bs_put(self.current_close[x.underlying], x.strike, (self.days_to_expiry[x.expiry] - days_since_start)/252, Config.interest_rate, x.vol), axis = 1)

                vols = pd.concat([vols_calls, vols_puts])
                vols.dropna(inplace = True)
                price_dict = vols.set_index('tradingsymbol').price.to_dict()

            else:
                print(Fore.YELLOW + "Vol Tracker Empty")
                print(Style.RESET_ALL)
                price_dict = None
            
        except Exception as e:            
            print(Fore.RED +f"{e}, Vol Pricing Error")
            print(Style.RESET_ALL)
            price_dict = None
        
        return price_dict
    
    def _get_price_packet(self):

        price_packet = {}
        tries = 5

        while tries > 0:

            try:
                price_packet_dataframe = pd.DataFrame(client.Live_Trading.Price_Packet.find())
                if len(price_packet_dataframe) == 0:
                    raise RuntimeError("Empty Price Packet")
                else:
                    break
                    
            except Exception as e:
                tries -= 1
                if tries == 0:
                    tries -= 1
                    print(Fore.RED, f"{e} Could Not Get Price Packet From DB")
                    print(Style.RESET_ALL)
                    break
                    
                else:
                    if tries > 2 and tries <= 3:
                        print(Fore.GREEN, f"{e} Problem In Retrieving Price Pakcet From DB, Trying Again (tries_left: {tries})")
                        print(Style.RESET_ALL)
                        
                    elif tries <= 2:
                        print(Fore.YELLOW, f"{e} Problem In Retrieving Price Pakcet From DB, Trying Again (tries_left: {tries})")
                        print(Style.RESET_ALL)
                        
                    time.sleep(1)
                    
        if tries <= 0:
            price_packet = None

        else:
            
            try:
                price_packet_dataframe = price_packet_dataframe.drop(columns = ['_id']).set_index('instrument').astype(float).fillna(0).reset_index().to_dict('records')
                for packet in price_packet_dataframe:

                    if packet["instrument"] in self.underlying:
                        price_packet[packet["instrument"]] = packet["last_price"]

                    else:
                        price_packet[packet["instrument"]] = {'last_price': packet["last_price"],
                                                              'depth' : {
                                                                  'buy' : [{'price' : packet["buy_price_1"], 'quantity': packet["buy_quantity_1"]}, 
                                                                           {'price' : packet["buy_price_2"], 'quantity': packet["buy_quantity_2"]},
                                                                           {'price' : packet["buy_price_3"], 'quantity': packet["buy_quantity_3"]},
                                                                           {'price' : packet["buy_price_4"], 'quantity': packet["buy_quantity_4"]},
                                                                           {'price' : packet["buy_price_5"], 'quantity': packet["buy_quantity_5"]}],
                                                                  'sell' : [{'price' : packet["sell_price_1"], 'quantity': packet["sell_quantity_1"]}, 
                                                                           {'price' : packet["sell_price_2"], 'quantity': packet["sell_quantity_2"]},
                                                                           {'price' : packet["sell_price_3"], 'quantity': packet["sell_quantity_3"]},
                                                                           {'price' : packet["sell_price_4"], 'quantity': packet["sell_quantity_4"]},
                                                                           {'price' : packet["sell_price_5"], 'quantity': packet["sell_quantity_5"]}]}
                                                             }
            
            except Exception as e:
                print(Fore.RED, f"{e}, Error In Processing Downloaded Price Packet")
                print(Style.RESET_ALL)
                price_packet = None
        
        #checking if there is incomplete price packet
        if price_packet is not None:
            for instrument in self.underlying + self.instrument_list.tradingsymbol.tolist():
                if instrument not in price_packet:
                    price_packet = None
                    break
                    
        return price_packet

    def _get_order_ack(self):

        #order_acks -> 'tradingsymbol', 'underlying', 'strategy_variant', 'quantity', 'order_id', 'price', 'ack_number'
        try:
            
            tries_left = 5
            while tries_left != 0:
                tries_left -= 1
                try:
                    order_acks = pd.DataFrame(client.Live_Trading.Order_Acks.find())
                    break
                    
                except Exception as e:
                    time.sleep(1)
                    print(Fore.RED, f"{e}, Trying Again To Retrieve Order Acks (tries_left: {tries})")
                    print(Style.RESET_ALL)
                    if tries_left == 0:
                        raise RuntimeError(e)

            if len(order_acks) != 0:
                
                order_acks.drop(columns=['_id'], inplace = True)
                order_acks = order_acks[order_acks.ack_number > self.order_ack_number]
                if len(order_acks) != 0:
                    
                    self.order_ack_number = order_acks.ack_number.max()
                    print(Fore.BLUE, Back.LIGHTWHITE_EX, f"Received Order Acks At {datetime.datetime.now()}")
                    print(order_acks)
                    print(Style.RESET_ALL)
                    
        except Exception as e:
            print(Fore.RED + f"{e}, Problem With Reading Order Acks")
            print(Style.RESET_ALL)
            order_acks = None
        
        return order_acks
    
    def _send_order_request(self, responses):
        
        try:
            order_requests = pd.concat(responses)
            order_requests = order_requests[['tradingsymbol', 'underlying', 'strategy_variant', 'quantity', 'priority']]

            self.order_req_number += 1
            order_requests["req_number"] = self.order_req_number
            
            tries_left = 5
            while tries_left != 0:
                tries_left -= 1
                try:
                    client.Live_Trading.Order_Requests.insert_many(order_requests.to_dict('records'))
                    print(Fore.BLACK, Back.LIGHTGREEN_EX, f"Sent These Order Acks At {datetime.datetime.now()}")
                    print(order_requests)
                    print(Style.RESET_ALL)
                    break
                    
                except Exception as e:
                    time.sleep(1)
                    print(Fore.YELLOW, f"{e}, Trying Again To Send These Orders (tries_left: {tries})")
                    print(Style.RESET_ALL)
                    print(order_requests)
                    if tries_left == 0:
                        raise RuntimeError(e)
                
        except Exception as e:
            print(Fore.RED + f"{e}, Could Not Send These Orders")
            print(Style.RESET_ALL)
            print(order_requests)
    
    def _get_index(self):
        
        try:
            index_price = {}
            #underlying, price, timestamp
            tries_left = 5
            while tries_left != 0:
                tries_left -= 1
                try:
                    index = pd.DataFrame(client.Live_Trading.Index_Tracker.find())
                    break
                except Exception as e:
                    print(Fore.YELLOW, f"{e}, Could Not Read From Index Tracker, Trying Again (tries_left: {tries})")
                    print(Style.RESET_ALL)
                    time.sleep(1)
                    if tries_left == 0:
                        raise RuntimeError(e)

            if len(index) != 0:

                for underlying in self.underlying:

                    temp = index[index.underlying == underlying].sort_values('timestamp', ascending = False)
                    self._momentum_indicator(underlying, temp.price.tolist())
                    if len(temp) != 0:
                        index_price[underlying] = temp.price.tolist()[0]
                    else:
                        index_price[underlying] = self.current_close[underlying]

            else:
                print(Fore.YELLOW + "Index Tracker Empty")
                print(Style.RESET_ALL)
                index_price = None
                
        except Exception as e:
            print(Fore.RED + f"{e}, Error In Retrieving Index Levels")
            print(Style.RESET_ALL)
            index_price = None
        
        self._send_momentum_indicator_to_db()
        return index_price
    
    def _momentum_indicator(self, underlying, price_list):
        
        try:
            start_price  = price_list[self.momentum_lags[underlying]]
            end_price = price_list[0]
            percent_change = 1e4 * (end_price - start_price) / start_price
            
            if percent_change >= self.momentum_threshold[underlying]:
                self.momentum_indicator[underlying] = self.momentum_switch
                
            elif percent_change <= -self.momentum_threshold[underlying]:
                self.momentum_indicator[underlying] = -self.momentum_switch
                
            else:
                self.momentum_indicator[underlying] = 0

        except Exception as e:
            self.momentum_indicator[underlying] = 0
    
    def _send_momentum_indicator_to_db(self):
        
        tries_left = 5
        while tries_left != 0:

            tries_left -= 1
            try:
                client.Live_Trading.Momentum_Indicator.delete_many({})
                client.Live_Trading.Momentum_Indicator.insert_many(pd.DataFrame({'underlying' : list(self.momentum_indicator.keys()), 'momentum_indicator' : list(self.momentum_indicator.values())}).to_dict('records'))
                break
                
            except Exception as e:
                
                time.sleep(0.1)
                if tries_left != 0:
                    print(Fore.YELLOW, f"{e}, Could Not Insert Into Momentum_Indicator, Trying Again (tries_left: {tries_left})")
                    print(Style.RESET_ALL)

                else:                    
                    print(Fore.RED, f"{e}, Could Not Insert Into Momentum_Indicator, Failed")
                    print(Style.RESET_ALL)

    def _revise_capital_allocation(self):

        print(Back.LIGHTCYAN_EX, Fore.BLACK)
        sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
        capital = 0.975 * float(input("Enter Total Capital Again? ").strip()) / len(self.tradeable_strategies)

        for strategy_identifier in self.strategizers:
            self.strategizers[strategy_identifier].allocated_capital = capital

        print(f"Capital Allocation Updated : {capital} Per Strategy")
        print(Style.RESET_ALL)

    def Start(self):
        
        minute_flag = False
        while self.current_batch_id <= 344:
            
            try:
                #skeleton of input provided to strategizers
                inputs = {'price_packet' : None, 'price_guides' : None, 'minute_flag' : None, 
                'orders' : None, 'update_greeks' : False, 'momentum_indicator' : None}

                #check timestamp first
                minutes_since_start = (datetime.datetime.now().timestamp() - datetime.datetime.strptime(f"{self.date_today}" + " 09:15:00", "%Y-%m-%d %H:%M:%S").timestamp()) / 60
                if minutes_since_start < 0:
                    print(Fore.YELLOW, f"Market Not Open, Time Now: {datetime.datetime.now()}")
                    print(Style.RESET_ALL)
                    time.sleep(30)
                    continue            

                #get index levels
                temp = self._get_index()
                if temp is None:
                    continue
                else:
                    self.current_close = copy.deepcopy(temp)
                
                #updating momentum_indicator in inputs
                inputs["momentum_indicator"] = self.momentum_indicator
                
                #get price packet
                price_packet = self._get_price_packet()
                if price_packet is None:
                    continue            
                
                #get price guides
                price_guides = self._get_price_guide()
                if price_guides is None:
                    continue

                # set additional fields in price guides and price packets
                for index, price in self.current_close.items():
                    price_packet[index] = price
                    price_guides[index] = price

                #updating price_guides in inputs
                inputs["price_guides"] = price_guides

                #get order acks and update orders in strategizers
                order_acks = self._get_order_ack()
                if order_acks is None:
                    continue

                elif len(order_acks) != 0:

                    order_acks["strategy_identifier"] = order_acks.strategy_variant.astype(str) + order_acks.underlying.astype(str)
                    for strategy_identifier in self.strategizers:

                        order_exec = order_acks[order_acks.strategy_identifier == strategy_identifier]
                        if len(order_exec) == 0:
                            continue
                        else:
                            order_exec = order_exec[["tradingsymbol", "quantity", "price", "order_id"]]
                            order_exec.timestamp = datetime.datetime.today()
                            inputs['orders'] = order_exec.to_dict('records')
                            response = self.strategizers[strategy_identifier].Update(inputs)

                #set minute flag, batch id and 
                if np.trunc(minutes_since_start) > self.current_batch_id:
                    self.current_batch_id += 1
                    minute_flag = True
                    price_packet["batch_id"] = self.current_batch_id

                else:
                    minute_flag = False
                    price_packet["batch_id"] = -1

                #preparing inputs for strategy signal generator update
                inputs["price_packet"] = price_packet
                inputs["minute_flag"] = minute_flag
                inputs["orders"] = None
                
                #updating strategy in each strategizers
                order_requests = []
                for strategy_identifier in self.strategizers:

                    response = self.strategizers[strategy_identifier].Update(inputs)                        
                    self.pnl[strategy_identifier]["realized_pnl"] = self.strategizers[strategy_identifier].pnl
                    
                    if response['orders_to_execute'] is not None:
                        order_requests.append(response['orders_to_execute'])
                    
                    if response["portfolio_log"] is not None:
                        self.pnl[strategy_identifier]["un-realized_pnl"] = response["portfolio_log"]["portfolio_pnl"]
                    else:
                        self.pnl[strategy_identifier]["un-realized_pnl"] = 0

                #sending order requests
                if len(order_requests) != 0:
                    self._send_order_request(order_requests)
                
                #displaying performance
                if minute_flag:
                    
                    print(Fore.WHITE, Back.LIGHTBLUE_EX)
                    print(f"PnL Summary for batch_id {self.current_batch_id} At {datetime.datetime.now()}")                    
                    print(Style.RESET_ALL)
                    
                    for strategy_identifier, pnl_summary in self.pnl.items():
                        
                        print(Fore.LIGHTMAGENTA_EX, Back.BLACK)
                        strategy_variant = self.strategizers[strategy_identifier].parameters["strategy_variant"]
                        
                        if (strategy_variant - 1) % 24 > 11:
                            current_week = False
                        else:
                            current_week = True
                        
                        if (strategy_variant - 1) % 12 > 5:
                            aggressive_hedge = True
                        else:
                            aggressive_hedge = False
                        
                        if (strategy_variant - 1) % 6 > 2:
                            mean_reversion = True
                        else:
                            mean_reversion = False
                        
                        print(f"{strategy_identifier} : {pnl_summary}, Trade Count : {self.strategizers[strategy_identifier].portfolio_id}")
                        print(f"Current State : {self.strategizers[strategy_identifier].portfolio_state}, Complete Fill : {self.strategizers[strategy_identifier].complete_fill}, Drawdowns : {self.strategizers[strategy_identifier].drawdown_count}")
                        print(f"Take Profit : {self.strategizers[strategy_identifier].take_profit}, Book Hedge : {self.strategizers[strategy_identifier].book_hedge}, Stop Loss : {self.strategizers[strategy_identifier].stop_loss}")
                        print(f"Current Week : {current_week}, Aggressive Hedging : {aggressive_hedge}, Mean Reverting : {mean_reversion}, Position Ratio : ", self.strategizers[strategy_identifier].parameters["strategy_params"]["position_ratio"])
                        print(Style.RESET_ALL)
                        
                        self.strategizers[strategy_identifier].Pretty_Print()
                    
                    if self.current_batch_id == 9:
                        self._revise_capital_allocation()
                        
            except RuntimeError as e:
                print(Fore.RED + f"Expected Error : {e}")
                if minute_flag:
                    self.current_batch_id -= 1

print(Fore.BLUE, Back.LIGHTWHITE_EX, "Incoming Message")
print(Fore.LIGHTMAGENTA_EX, Back.BLACK, "Class Variables")
print(Fore.BLACK, Back.LIGHTYELLOW_EX, "Internal Variables")
print(Fore.BLACK, Back.LIGHTGREEN_EX, "Outgoing Message")
print(Style.RESET_ALL)

print(Back.LIGHTCYAN_EX, Fore.BLACK)
sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
parameters = {"date_today" : input("Enter Today's Date? ").strip(), 
              "date_yesterday" : input("Enter Yesterday's Date? ").strip(), 
              "underlying" : ['NIFTY'], 
              "capital" : float(input("Enter Total Capital? ").strip()),
              "momentum_threshold" : {"NIFTY" : 9}, #in bps
              "momentum_lags" : {"NIFTY" : 84}, #number of seconds to look back to judge momentum
              "momentum_switch" : {"NIFTY" : -1}} #1-> momentum, -1 -> mean reverting

#parameters = {"date_today" : input("Enter Today's Date? ").strip(), 
#              "date_yesterday" : input("Enter Yesterday's Date? ").strip(), 
#              "underlying" : ['NIFTY', 'BANKNIFTY'], 
#              "capital" : float(input("Enter Total Capital? ").strip()),
#              "momentum_threshold" : {"NIFTY" : 5, "BANKNIFTY" : 5}, #in bps
#              "momentum_lags" : {"NIFTY" : 41, "BANKNIFTY" : 41}, #number of seconds to look back to judge momentum
#              "momentum_switch" : {"NIFTY" : 1, "BANKNIFTY" : 1}} #1-> momentum, -1 -> mean reverting


print(Style.RESET_ALL)

print(Fore.LIGHTMAGENTA_EX, Back.BLACK)
print(f"parameters")
for key, value in parameters.items():
    print(f"{key} : {value}, type : {type(value)}")
print(Style.RESET_ALL)

self = Strategy_Central_Command(parameters)
self.Start()