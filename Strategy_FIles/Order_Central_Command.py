import API_Wrapper
import Config
import BS
import os
import time
import datetime
import numpy as np
import pandas as pd
pd.set_option('display.max_columns', None)
pd.set_option('display.max_rows', None)

import random
import copy
from colorama import Fore, Back, Style
import warnings
warnings.filterwarnings("ignore")

from kiteconnect import KiteConnect
from pymongo import MongoClient
client = MongoClient(Config.DB_Hostname, Config.DB_Port)

class Order_Central_Command:
    
    def __init__(self, parameters, trading_parameters):

        #cleaning DataBase
        print(client.Live_Trading.Order_Acks.delete_many({}).deleted_count,f" Orders Cleared From Order Acks DB")
        print(client.Live_Trading.Order_Requests.delete_many({}).deleted_count,f" Orders Cleared From Order Requests DB")
        
        self.trading_parameters = copy.deepcopy(trading_parameters)
        #{'paper_trading' = False, 'price_tolerance' : 0.02, 'price_trail' : 0.01, 'max_mod_count_for_price_trail' : 5, 
        # 'min_price_change_before_mod_until_price_trail' : 0.005, 'min_seconds_before_mod' : 10}
        
        self.parameters = copy.deepcopy(parameters)
        #parameters = {'underlying' : ['NSE:NIFTY 50', 'NSE:NIFTY BANK'], 
        #'symbol_map' : {"NSE:NIFTY 50" : "NIFTY", "NSE:NIFTY BANK" : "BANKNIFTY"},
        #'reversed_symbol_map' : {"NIFTY" : "NSE:NIFTY 50", "BANKNIFTY" : "NSE:NIFTY BANK"},
        #'date_today':'2023-09-27'}
        
        #initialize ack and order request number
        self.order_req_number = 0
        self.order_ack_number = 0
        
        #initialize instruments to price
        self.focus_instruments =[]
        
        #initialize ordertrackers
        self.order_request_tracker = pd.DataFrame()
        #order_request_tracker = pd.DataFrame()
        #columns='tradingsymbol','underlying','strategy_variant','quantity','priority','tag','filled_quantity','status'
        #status -> 1 -> placed, 0 -> no orders placed
        self.filled_order_tracker = {}
        self.unfilled_order_tracker = {}
        #order_tracker = {"<order_id>":{"tradingsymbol":, "absolute_quantity":, "direction":, "price":, "last_updated":<epoch time>, "iceberg_flag":True/False, "tag":, parent_order_id:}}
        
        #tag_map
        self.tag_map = {} #{"<bad tag>" : "<proper tag>"} or {"<proper tag>" : "<proper tag>"}

        #initialize APIs
        self.API_Interface = API_Wrapper.API_Wrapper(market_adapter = False, paper_trading = self.trading_parameters['paper_trading'])
        
        #initialize instrument list
        self.instrument_list = None
        self.instrument_underlying_map = None
        self.instrument_expiry_map = None
        self._get_instrument_list()

        #initialize index price dictionary and price guides
        self.previous_prices = {}
        self.current_prices = {}
        self.price_guides = None
        self._get_prices()
        
        #initialize days to expiry and fix trading parameters
        self.days_to_expiry = {}
        self._get_days_to_expiry()
        
        #initialize momentum indicators
        self.momentum_indicators = None
                
        print(Fore.GREEN, "Order Central Command Initiated")
        print(Style.RESET_ALL)

    def _get_instrument_list(self):
        
        self.instrument_list = pd.DataFrame(client.Live_Trading.Instrument_Universe.find()).drop(columns=['_id'])
        if len(self.instrument_list)==0:
            raise RuntimeError(Fore.RED + "Empty Instrument List")
            
        underlying_list = [self.parameters['symbol_map'][underlying] for underlying in self.parameters['underlying']]
        if len(set(underlying_list + self.instrument_list.underlying.tolist())) != len(underlying_list):
            raise RuntimeError(Fore.RED + "Empty / Improper Instrument List")
        
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
        
        self.instrument_underlying_map = self.instrument_list.set_index('tradingsymbol').underlying.to_dict()
        self.instrument_expiry_map = self.instrument_list.set_index('tradingsymbol').expiry.to_dict()
        
    def _to_market_suitable_price(self, price):
        return max(round(round(price / Config.tick_size) * Config.tick_size, 2), Config.tick_size)
        
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
                            print(Style.RESET_ALL)
                            if self.parameters['symbol_map'][instrument] in self.previous_prices:
                                self.current_prices[self.parameters['symbol_map'][instrument]] = self.previous_prices[self.parameters['symbol_map'][instrument]]
                            else:
                                tries = 5
                                while tries > 0:
                                    try:
                                        sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
                                        self.current_prices[self.parameters['symbol_map'][instrument]] = float(input(Back.LIGHTCYAN_EX + Fore.RED + f"Enter Price For {instrument} "))
                                        print(Style.RESET_ALL)
                                        break
                                    except Exception as e:
                                        tries -= 1
                                        if tries == 0:
                                            raise RuntimeError(Fore.RED + f"{e}, Invalid Price!!")
                                        else:
                                            print(Fore.RED + f"{e}, Error Try Again (tries_left: {tries})")
                                            print(Style.RESET_ALL)

                    else:
                        self.current_prices[self.parameters['symbol_map'][instrument]] = quotes[instrument]

                else:
                    print(Fore.YELLOW + f"{instrument} quote unavailable")
                    print(Style.RESET_ALL)
                    if self.parameters['symbol_map'][instrument] in self.previous_prices:
                        self.current_prices[self.parameters['symbol_map'][instrument]] = self.previous_prices[self.parameters['symbol_map'][instrument]]
                    elif instrument in self.parameters['underlying']:
                        tries = 5
                        while tries > 0:
                            try:
                                sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
                                self.current_prices[self.parameters['symbol_map'][instrument]] = float(input(Back.LIGHTCYAN_EX + Fore.RED + f"Enter Price For {instrument} "))
                                print(Style.RESET_ALL)
                                break
                            except Exception as e:
                                tries -= 1
                                if tries == 0:
                                    raise RuntimeError(Fore.RED + f"{e}, Invalid Price!!")
                                else:
                                    print(Fore.RED + f"{e}, Error Try Again (tries_left: {tries})")
                                    print(Style.RESET_ALL)

                    else:
                        self.current_prices[self.parameters['symbol_map'][instrument]] = None

            self.previous_prices = copy.deepcopy(self.current_prices)

        except Exception as e:
            self.current_prices = copy.deepcopy(self.previous_prices)
            print(Fore.RED, f"{e}, Copying previous prices")
            print(Style.RESET_ALL)
        
        #preparing price_guides
        try:
            focus_instruments = [self.parameters['symbol_map'][instrument] for instrument in self.focus_instruments]
            tries = 5
            while tries > 0:
                try:
                    vols = pd.DataFrame(client.Live_Trading.Vol_Tracker.find({"tradingsymbol":{"$in":focus_instruments}}))
                    break
                except Exception as e:
                    tries -= 1
                    if tries == 0:
                        print(Fore.RED, f"{e}, Could Not Retrieve Vols, Failed")
                        print(Style.RESET_ALL)
                        break
                    else:
                        print(Fore.YELLOW, f"{e}, Could Not Retrieve Vols, Trying Again (tries_left: {tries})")
                        print(Style.RESET_ALL)
                        time.sleep(1)
                        
            if len(vols) != 0:
                
                vols.drop(columns=['_id'], inplace = True)
                vols = vols[vols.seq_no > vols.seq_no.max() - 10]
                vols = vols.groupby('tradingsymbol').mean()[['vol']].reset_index()
                vols["strike"]= vols.tradingsymbol.map(self.instrument_list.set_index('tradingsymbol').strike.to_dict())
                vols["underlying"] = vols.tradingsymbol.map(self.instrument_list.set_index('tradingsymbol').underlying.to_dict())
                vols["instrument_type"] = vols.tradingsymbol.map(self.instrument_list.set_index('tradingsymbol').instrument_type.to_dict())
                vols["expiry"] = vols.tradingsymbol.map(self.instrument_list.set_index('tradingsymbol').expiry.to_dict())

                days_since_start = (datetime.datetime.now().timestamp() - datetime.datetime.strptime(f"{self.parameters['date_today']}" + " 09:15:00", "%Y-%m-%d %H:%M:%S").timestamp()) / 22500 
                days_since_start = max(0, min(days_since_start, 0.96))
                vols_calls = vols[vols.instrument_type=="CE"]
                vols_puts = vols[vols.instrument_type=="PE"]
                vols_calls["price"] = vols_calls.apply(lambda x : BS.bs_call(self.current_prices[x.underlying], x.strike, (self.days_to_expiry[x.expiry] - days_since_start)/252, Config.interest_rate, x.vol), axis = 1)
                vols_puts["price"] = vols_puts.apply(lambda x : BS.bs_put(self.current_prices[x.underlying], x.strike, (self.days_to_expiry[x.expiry] - days_since_start)/252, Config.interest_rate, x.vol), axis = 1)

                vols = pd.concat([vols_calls, vols_puts])
                vols.dropna(inplace = True)
                self.price_guides = vols.set_index('tradingsymbol').price.to_dict()
                
                for tradingsymbol, price in self.price_guides.items():
                    self.price_guides[tradingsymbol] = self._to_market_suitable_price(price)
                    
            elif len(focus_instruments) != 0:
                print(Fore.YELLOW + "Vol Tracker Might Not Contain Focus Instruments")
                print(Style.RESET_ALL)
                self.price_guides = None

        except Exception as e:            
            print(Fore.RED +f"{e}, Vol Pricing Error")
            print(Style.RESET_ALL)
            self.price_guides = None

    def _get_days_to_expiry(self):
        
        days_to_expiry = pd.DataFrame(client.Strategy.Days_To_Expiry.find({"date" : self.parameters['date_today']})).drop(columns=['_id','date']).to_dict('records')
        days_to_expiry = {temp['underlying']: {'current_week': temp['current_week'], 'next_week': temp['next_week']} for temp in days_to_expiry}
        trading_parameters_cols_to_fix = ["price_tolerance", "price_trail", "max_mod_count_for_price_trail", "min_price_change_before_mod_until_price_trail", "min_price_change_before_mod_after_price_trail", "min_seconds_before_mod"]
        
        for underlying in self.parameters["underlying"]:

            opt_expiry = sorted(self.instrument_list[(self.instrument_list.underlying == self.parameters["symbol_map"][underlying])&(self.instrument_list.instrument_type == "CE")].expiry.unique())
            
            if len(opt_expiry) < 2:
                print(Fore.RED, f"Couldnt Find Two Consequtive Expiries For {underlying}")
                print(Style.RESET_ALL)
                continue
                
            self.days_to_expiry[opt_expiry[0]] = days_to_expiry[self.parameters["symbol_map"][underlying]]["current_week"]
            self.days_to_expiry[opt_expiry[1]] = days_to_expiry[self.parameters["symbol_map"][underlying]]["next_week"]
            
            for key in trading_parameters_cols_to_fix:
                self.trading_parameters[key][self.parameters["symbol_map"][underlying]][opt_expiry[0]] = self.trading_parameters[key][self.parameters["symbol_map"][underlying]].pop("current_week")
                self.trading_parameters[key][self.parameters["symbol_map"][underlying]][opt_expiry[1]] = self.trading_parameters[key][self.parameters["symbol_map"][underlying]].pop("next_week")
    
    def _get_momentum_indicators(self):
        
        tries_left = 5
        while tries_left != 0:

            tries_left -= 1
            try:
                self.momentum_indicators = pd.DataFrame(client.Live_Trading.Momentum_Indicator.find()).drop(columns = ['_id']).set_index('underlying').momentum_indicator.to_dict()
                break

            except Exception as e:

                time.sleep(0.1)
                if tries_left != 0:
                    print(Fore.YELLOW, f"{e}, Could Not Read From Momentum_Indicator, Trying Again (tries_left: {tries_left})")
                    print(Style.RESET_ALL)

                else:                    
                    print(Fore.RED, f"{e}, Could Not Read From Momentum_Indicator, Failed")
                    print(Style.RESET_ALL)
                    self.momentum_indicators = None

    def _get_order_request(self):
        
        try:
            tries = 5
            while tries > 0:
                try:
                    order_reqs = pd.DataFrame(client.Live_Trading.Order_Requests.find())
                    break
                except Exception as e:
                    tries -= 1
                    if tries == 0:
                        print(Fore.RED, f"{e}, Could Not Retrieve Order Requests, Failed")
                        print(Style.RESET_ALL)
                        order_reqs = pd.DataFrame()
                        break
                    else:
                        print(Fore.YELLOW, f"{e}, Could Not Retrieve Order Requests, Trying Again (tries_left: {tries})")
                        print(Style.RESET_ALL)
                        time.sleep(1)

            if len(order_reqs) != 0:
                
                order_reqs = order_reqs[['tradingsymbol', 'underlying', 'strategy_variant', 'quantity', 'priority', 'req_number']]
                order_reqs.req_number = order_reqs.req_number.astype(int)
                order_reqs = order_reqs[order_reqs.req_number > self.order_req_number]
                
                if len(order_reqs) != 0:
                    
                    print(Fore.BLUE, Back.LIGHTWHITE_EX)
                    print(f"Received New Order Requests At {datetime.datetime.now()}")
                    print(order_reqs)
                    print(Style.RESET_ALL)
                    
                    self.order_req_number = order_reqs.req_number.max()
                    order_reqs.drop(columns=['req_number'], inplace = True)
                    order_reqs.tradingsymbol = order_reqs.tradingsymbol.astype(str)
                    order_reqs.underlying = order_reqs.underlying.astype(str)
                    order_reqs.strategy_variant = order_reqs.strategy_variant.astype(int)
                    order_reqs.quantity = order_reqs.quantity.astype(int)
                    order_reqs.priority = order_reqs.priority.astype(int)
                    order_reqs["tag"] = order_reqs.strategy_variant.astype(str) + order_reqs.underlying.astype(str)
                    order_reqs["filled_quantity"] = 0
                    order_reqs["status"] = 0
                    
                    #fill tag map
                    for tag in order_reqs.tag.unique():
                        if tag not in self.tag_map:
                            self.tag_map[tag] = tag
                            
                    focus_instruments = order_reqs.tradingsymbol.unique()
                    for instrument in focus_instruments:
                        
                        if instrument in self.parameters['reversed_symbol_map']:
                            continue
                            
                        else:
                            instrument_temp = "NFO:" + instrument
                            self.focus_instruments.append(instrument_temp)
                            self.parameters['symbol_map'][instrument_temp] = instrument
                            self.parameters['reversed_symbol_map'][instrument] = instrument_temp
                    
                    if len(self.order_request_tracker) == 0:    
                        self.order_request_tracker = copy.deepcopy(order_reqs)
                    else:
                        self.order_request_tracker = pd.concat([order_reqs, self.order_request_tracker])
                    
                    print(Fore.LIGHTMAGENTA_EX, Back.BLACK, f"Order Request Tracker Status After Receiving Order Requests At {datetime.datetime.now()}")
                    print(self.order_request_tracker)
                    print(Style.RESET_ALL)
                    
                else:
                    pass
                    #print(Fore.YELLOW + f"No New Requests. Last Request Number : {self.order_req_number}")
                    #print(Style.RESET_ALL)

            else:
                pass
                #print(Fore.YELLOW + "No Order Requests")

        except Exception as e:
            print(Fore.RED + f"{e}, Problem With Reading Order Requests")
            print(Style.RESET_ALL)

    def _send_orders(self):
        
        order_placed_flag = False
        order_request_tracker_post_sending_orders = None
        if len(self.order_request_tracker) !=0 :
            
            for tag in self.order_request_tracker.tag.unique():
                
                #segregating orders_requests by stratgegy
                strategy_requests = self.order_request_tracker[self.order_request_tracker.tag == tag]
                
                #segregating straegy level orders_requests that have not been placed
                placed_orders = strategy_requests[strategy_requests.status == 1]
                unplaced_orders = strategy_requests[strategy_requests.status == 0]
                
                #count of high priority unfilled orders
                high_priority_count = len(strategy_requests[strategy_requests.priority == 0])
                
                #segregating placed orders by priority
                high_priority_unplaced_orders = unplaced_orders[unplaced_orders.priority == 0]
                low_priority_unplaced_orders = unplaced_orders[unplaced_orders.priority == 1]
                
                #dealing orders by priority
                if len(high_priority_unplaced_orders) != 0:
                    
                    #updating prices
                    self._get_prices()
                    
                    high_priority_unplaced_orders_list =  high_priority_unplaced_orders.to_dict('records')
                    high_priority_unplaced_orders_list_after_processing = []
                    
                    for order in high_priority_unplaced_orders_list:
                        high_priority_unplaced_orders_list_after_processing.append(self._process_orders(order))
                        order_placed_flag = True
                    
                    high_priority_unplaced_orders = pd.DataFrame(high_priority_unplaced_orders_list_after_processing)
                
                elif len(low_priority_unplaced_orders) != 0 and high_priority_count == 0:
                    
                    #updating prices
                    self._get_prices()

                    low_priority_unplaced_orders_list =  low_priority_unplaced_orders.to_dict('records')
                    low_priority_unplaced_orders_list_after_processing = []
                    
                    for order in low_priority_unplaced_orders_list:
                        low_priority_unplaced_orders_list_after_processing.append(self._process_orders(order))
                        order_placed_flag = True
                    
                    low_priority_unplaced_orders = pd.DataFrame(low_priority_unplaced_orders_list_after_processing)
                    
                # stitching back everything
                unplaced_orders = pd.concat([high_priority_unplaced_orders, low_priority_unplaced_orders])
                strategy_requests = pd.concat([placed_orders, unplaced_orders])
                if order_request_tracker_post_sending_orders is None:
                    order_request_tracker_post_sending_orders = copy.deepcopy(strategy_requests)
                else:
                    order_request_tracker_post_sending_orders = pd.concat([order_request_tracker_post_sending_orders, strategy_requests])
            
            #copying back processed strategy_requests
            self.order_request_tracker = copy.deepcopy(order_request_tracker_post_sending_orders)
            if order_placed_flag:
                print(Fore.LIGHTMAGENTA_EX, Back.BLACK, f"Order Request Tracker Status After Placing Orders At {datetime.datetime.now()}")
                print(self.order_request_tracker)
                print(Style.RESET_ALL)

    def _process_orders(self, order):
        
        #order_tracker = {"<order_id>":{"tradingsymbol":, "absolute_quantity":, "direction":, "price":, "last_updated":<epoch time>, "iceberg_flag":True/False, "tag"}}
        quantity_remaining = order["quantity"]
        price = self._get_execution_price(order["tradingsymbol"], np.sign(quantity_remaining), trail = True)
        tries_left = 10
        
        while quantity_remaining != 0:
            
            if abs(quantity_remaining) <= Config.qty_limit_per_order[order["underlying"]]:
                
                qty_placed = quantity_remaining
                order_to_place = {"tradingsymbol": order["tradingsymbol"], "quantity": qty_placed, "tag": order["tag"], "price": price, "iceberg_legs": None, "iceberg_quantity": None}
                order_id = self.API_Interface.Place_Order(order_to_place)
                            
            else:
                
                qty_placed = np.sign(quantity_remaining) * min(Config.qty_limit_per_order[order["underlying"]] * Config.kite_iceberg_leg_limit, abs(quantity_remaining))
                iceberg_legs, iceberg_quantity = self._get_iceberg_legs_and_qty(qty_placed, Config.lot_size[order["underlying"]], Config.qty_limit_per_order[order["underlying"]])
                order_to_place = {"tradingsymbol": order["tradingsymbol"], "quantity": qty_placed, "tag": order["tag"], "price": price, "iceberg_legs": iceberg_legs, "iceberg_quantity": iceberg_quantity}
                order_id = self.API_Interface.Place_Order(order_to_place)

            if order_id > 0: 
                quantity_remaining -= qty_placed
                tries_left = 10
                print(Fore.BLACK, Back.LIGHTGREEN_EX, f"Successfully Placed This Order At {datetime.datetime.now()}")
                print(order_to_place)
                print(Style.RESET_ALL)

            else:
                tries_left -= 1
                print(f"tries_left: {tries_left}")
                
            if tries_left == 0:
                
                print(Fore.RED, f"Place Order For ", order["tradingsymbol"], f" With Qty {quantity_remaining} & Tag ", order["tag"], f" Possibly At {price}")
                sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
                user_input = input(Back.LIGHTCYAN_EX + Fore.GREEN + "Press Enter Once Done ")
                print(Style.RESET_ALL)
                quantity_remaining = 0
        
        order["status"] = 1
        return order
    
    def _get_execution_price(self, tradingsymbol, direction, trail = True):
        
        price_packet = {"buy" : self.current_prices[tradingsymbol]['depth']['buy'][0]['price'], "sell" : self.current_prices[tradingsymbol]['depth']['sell'][0]['price']}
        use_price_guide = False
        
        if tradingsymbol in self.price_guides:
            guided_price = self.price_guides[tradingsymbol]
        else:
            guided_price = None

        if direction > 0:
            market_price = {'target' : price_packet['buy'], 'fallback' : price_packet['sell']}
        else:
            market_price = {'target' : price_packet['sell'], 'fallback' : price_packet['buy']}
        
        try:    
            market_price['target'] /= 1
            
            if market_price['target'] == 0:
                raise RuntimeError("Zero Price Error")
            
            if direction * (market_price['target'] / guided_price - 1) >= self.trading_parameters['price_tolerance'][self.instrument_underlying_map[tradingsymbol]][self.instrument_expiry_map[tradingsymbol]]:
                use_price_guide = True
            
        except Exception as e:
            
            print(Fore.YELLOW, f"{e}, Improper Target Price For {tradingsymbol} & Direction {direction}")
            print(Style.RESET_ALL)
            try:
                market_price['target'] = market_price['fallback']
                market_price['target'] /= 1
                
                if market_price['target'] == 0:
                    raise RuntimeError("Zero Price Error")
                
                if direction * (market_price['target'] / guided_price - 1) >= self.trading_parameters['price_tolerance'][self.instrument_underlying_map[tradingsymbol]][self.instrument_expiry_map[tradingsymbol]]:
                    use_price_guide = True

            except Exception as e:
                print(Fore.YELLOW, f"{e}, Improper Fallback Price For {tradingsymbol} & Direction {direction}")
                print(Style.RESET_ALL)
                use_price_guide = True
        
        if use_price_guide:
            
            print(Fore.YELLOW, f"Using Price {guided_price} For {tradingsymbol} & Direction {direction}")
            print(Style.RESET_ALL)
            sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
            user_input = input(Back.LIGHTCYAN_EX + Fore.GREEN + "Enter Any Character If Otherwise ")
            print(Style.RESET_ALL)
            
            if len(user_input) == 0:
                market_price['target'] = guided_price
            else:
                tries = 5
                while tries > 0:
                    try:
                        sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
                        market_price['target'] = float(input(Back.LIGHTCYAN_EX + Fore.GREEN + "Enter Best Bid/Ask Price : "))
                        print(Style.RESET_ALL)
                        break
                    except Exception as e:
                        tries -= 1
                        if tries == 0:
                            raise RuntimeError(Fore.RED + f"{e}, Invalid Price!!")
                        else:
                            print(Fore.RED + f"{e}, Error Try Again (tries_left: {tries})")
                            print(Style.RESET_ALL)
        
        if trail:
            market_price['target'] *= (1 - direction * self.trading_parameters['price_trail'][self.instrument_underlying_map[tradingsymbol]][self.instrument_expiry_map[tradingsymbol]])
            market_price['target'] = self._to_market_suitable_price(market_price['target'])
        
        return market_price['target']
   
    def _get_iceberg_legs_and_qty(self, quantity, lot_size, max_order_limit):
        
        num_lots = abs(quantity) / lot_size
        max_lot_limit = max_order_limit / lot_size
        iceberg_legs = np.ceil(num_lots / max_lot_limit)
        iceberg_quantity = np.ceil((abs(quantity) / iceberg_legs) / lot_size) * lot_size
        return int(iceberg_legs), int(iceberg_quantity)
    
    def _prepare_dummy_order_acks(self):
        
        #tag
        #status OPEN/COMPLETE
        #order_id
        #tradingsymbol,
        #transaction_type BUY/SELL
        #price
        #average_price
        #variety iceberg/regular
        """
        self.order_request_tracker = pd.DataFrame()
        #order_request_tracker = pd.DataFrame()
        #columns='tradingsymbol','underlying','strategy_variant','quantity','priority','tag','filled_quantity','status'
        #status -> 1 -> placed, 0 -> no orders placed
        
        self.filled_order_tracker = {}
        self.unfilled_order_tracker = {}
        #order_tracker = {"<order_id>":{"tradingsymbol":, "absolute_quantity":, "direction":, "price":, "last_updated":<epoch time>, "iceberg_flag":True/False, "tag"}}
        """
        order_acks = []
        if len(self.order_request_tracker) == 0:
            order_acks =  pd.DataFrame()
            self.unfilled_order_tracker = {}
            
        elif np.random.randint(low=0, high = 2) == 1:
                        
            for tradingsymbol, underlying, strategy_variant, quantity, priority, tag, filled_quantity, status in \
            zip(self.order_request_tracker.tradingsymbol, self.order_request_tracker.underlying, self.order_request_tracker.strategy_variant, self.order_request_tracker.quantity, self.order_request_tracker.priority, self.order_request_tracker.tag, self.order_request_tracker.filled_quantity, self.order_request_tracker.status):
                                    
                if status == 1:
                    
                    random_order_id = np.random.randint(low=10, high= 1000000000)
                    
                    #random completion of filled 
                    if np.random.randint(low = 0, high = 2) == 1:
                        order_status = "COMPLETE"
                    else:
                        order_status = "OPEN"
                    
                    if abs(quantity - filled_quantity) > Config.qty_limit_per_order[underlying]:
                        quantity_to_place = Config.qty_limit_per_order[underlying] * np.sign(quantity - filled_quantity)
                        variety = "iceberg"
                        
                    else:
                        quantity_to_place = quantity - filled_quantity
                        variety = "regular"
                    
                    if quantity_to_place > 0:
                        transaction_type = "BUY"
                    else:
                        transaction_type = "SELL"

                    #random completion of filled 
                    if np.random.randint(low=0, high = 2) == 1:
                        tag += "_"
                    
                    order_acks.append({"tag" : tag, #
                                       "status" : order_status, #
                                       "order_id" : random_order_id, #
                                       "quantity" : quantity_to_place, #
                                       "tradingsymbol" : tradingsymbol, 
                                       "transaction_type" : transaction_type, #
                                       "price" : self.price_guides[tradingsymbol], 
                                       "average_price" : self.price_guides[tradingsymbol],
                                       "variety" : variety#
                                      })
        
        else:
            
            focus_instruments_strategy_wise = []
            for tradingsymbol, tag in zip(self.order_request_tracker.tradingsymbol, self.order_request_tracker.tag):
                focus_instruments_strategy_wise.append(tradingsymbol+tag)
            
            order_id_to_delete = []
            for order_id, attributes in self.unfilled_order_tracker.items():
                if attributes["tradingsymbol"] + attributes["tag"] in focus_instruments_strategy_wise:
                    order_id_to_delete.append(order_id)
            
            for order_id in order_id_to_delete:
                del self.unfilled_order_tracker[order_id]
            
            for order_id, attributes in self.unfilled_order_tracker.items():

                if np.random.randint(low = 0, high = 2) == 1:
                    order_status = "COMPLETE"
                else:
                    order_status = "OPEN"

                if attributes["iceberg_flag"]:
                    variety = "iceberg"
                else:
                    variety = "regular"
                
                if attributes["direction"] > 0:
                    transaction_type = "BUY"
                else:
                    transaction_type = "SELL"

                order_acks.append({"tag" : attributes["tag"],
                                   "status" : order_status,
                                   "order_id" : order_id,
                                   "quantity" : attributes["absolute_quantity"],
                                   "tradingsymbol" : attributes["tradingsymbol"], 
                                   "transaction_type" : transaction_type,
                                   "price" : self.price_guides[attributes["tradingsymbol"]], 
                                   "average_price" : self.price_guides[attributes["tradingsymbol"]],
                                   "variety" : variety
                                  })

            
        order_acks = pd.DataFrame(order_acks)
        time.sleep(20)
        return order_acks
        
    def _process_order_acks(self):
        
        #order_tracker = {"<order_id>":{"tradingsymbol":, "quantity":, "direction":, 
        #"price":, "last_updated":<epoch time>, "iceberg_flag":True/False, "tag":, "parent_order_id"}}
        if self.trading_parameters['paper_trading']:
            order_acks = self._prepare_dummy_order_acks()
        else:
            order_acks = self.API_Interface.Executions()
            
        if order_acks is not None and len(order_acks) != 0:
            
            #processing order acks
            for tag in order_acks.tag.unique():
                
                if tag not in self.tag_map:
                    
                    sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
                    user_input = input(Back.LIGHTCYAN_EX + Fore.RED + f"{tag} Tag Unmapped, Enter Appropriate Tag? (1 tries left)")
                    print(Style.RESET_ALL)
                    self.tag_map[tag] = user_input.strip().upper()
                    
            order_acks.tag = order_acks.tag.map(self.tag_map)
            
            #segregating filled and unfilled orders
            filled_orders = order_acks[order_acks.status == "COMPLETE"]
            unfilled_orders  = order_acks[order_acks.status == "OPEN"]
                        
            #checking if any new orders were filled
            filled_orders = filled_orders[~filled_orders.order_id.isin(self.filled_order_tracker)]
            if len(filled_orders) != 0:
                
                #filled order processing
                print(Fore.BLUE, Back.LIGHTWHITE_EX, f"Orders Filled At {datetime.datetime.now()}")
                print(filled_orders)
                print(Style.RESET_ALL)
                self._send_order_acks(filled_orders)                

                for order_id, tradingsymbol, absolute_quantity, direction, price, variety, tag, parent_order_id  in zip(filled_orders.order_id, filled_orders.tradingsymbol, filled_orders.quantity, filled_orders.transaction_type, filled_orders.price, filled_orders.variety, filled_orders.tag, filled_orders.parent_order_id):
                    
                    if order_id in self.unfilled_order_tracker:
                        
                        self.filled_order_tracker[order_id] = self.unfilled_order_tracker[order_id]
                        del self.unfilled_order_tracker[order_id]
                        
                    else:
                        
                        if variety.lower() == "iceberg":
                            iceberg_flag = True
                        else:
                            iceberg_flag = False
                            
                        if direction.upper() == "BUY":
                            direction = 1
                        else:
                            direction = -1
                            
                        self.filled_order_tracker[order_id] = {"tradingsymbol" : tradingsymbol,
                                                               "absolute_quantity" : abs(absolute_quantity),
                                                               "direction" : direction,
                                                               "price" : price,
                                                               "last_updated" : datetime.datetime.now().timestamp(),
                                                               "iceberg_flag" : iceberg_flag, 
                                                               "tag" : tag,
                                                               "parent_order_id" : parent_order_id}
                        
            #processing unfilled orders
            self._process_order_modifications(unfilled_orders)
        
    def _send_order_acks(self, filled_orders):
        
        #order_request_tracker
        #columns='tradingsymbol','underlying','strategy_variant','quantity','priority','tag','filled_quantity','status'
        #status -> 1 -> placed, 0 -> no orders placed
        
        order_acks_to_send = None
        for tag in self.order_request_tracker.tag.unique():
            
            #segregating orders_requests by stratgegy
            strategy_requests = self.order_request_tracker[self.order_request_tracker.tag == tag]
            strategy_requests["filled_quantity_temp"] = 0
            strategy_requests["price"] = 0
            if len(strategy_requests.tradingsymbol.unique()) != len(strategy_requests.tradingsymbol):
                raise RuntimeError(Fore.RED, "STOP EVERYTHING NOW!!! STRATEGIZERS ARE SENDING DUPLICATED ORDERS FOR INSTRUMENTS")
                
            #segregating filled orders for strategy
            filtered_filled_orders = filled_orders[filled_orders.tag == tag]
            
            if len(filtered_filled_orders) != 0:
                
                #converting quantity to positive negative sign
                filtered_filled_orders.quantity = np.where(filtered_filled_orders.transaction_type.str.upper() == "BUY", abs(filtered_filled_orders.quantity), -abs(filtered_filled_orders.quantity))
                
                #avg price and fills
                tradingsymbol_filled_quantity_map = filtered_filled_orders.groupby('tradingsymbol').sum().quantity.to_dict()
                tradingsymbol_average_price_map = filtered_filled_orders.assign(value_traded = abs(filtered_filled_orders.quantity * filtered_filled_orders.average_price)).groupby('tradingsymbol').sum().value_traded.to_dict()
                tradingsymbol_average_price_map = {symbol: tradingsymbol_average_price_map[symbol] / abs(quantity) for symbol, quantity in tradingsymbol_filled_quantity_map.items()}
                
                strategy_requests.filled_quantity_temp = np.where(strategy_requests.status == 1, strategy_requests.tradingsymbol.map(tradingsymbol_filled_quantity_map), 0)
                strategy_requests.price = strategy_requests.tradingsymbol.map(tradingsymbol_average_price_map)
                strategy_requests.price.fillna(0, inplace = True)
                strategy_requests.filled_quantity_temp.fillna(0, inplace = True)
                strategy_requests.filled_quantity += strategy_requests.filled_quantity_temp
                
            if order_acks_to_send is None:
                order_acks_to_send = copy.deepcopy(strategy_requests)
            else:
                order_acks_to_send = pd.concat([order_acks_to_send, strategy_requests])
                
        self.order_request_tracker = order_acks_to_send[order_acks_to_send.quantity != order_acks_to_send.filled_quantity].drop(columns = ['filled_quantity_temp', 'price'])
        order_acks_to_send = order_acks_to_send[order_acks_to_send.quantity == order_acks_to_send.filled_quantity][['tradingsymbol', 'underlying', 'strategy_variant', 'quantity', 'price']]
        
        if len(order_acks_to_send) != 0:
            self.order_ack_number += 1
            order_acks_to_send['ack_number'] = self.order_ack_number
            order_acks_to_send['order_id'] = random.sample(range(1, 1000000000), len(order_acks_to_send))
            
            while True:
                try:
                    client.Live_Trading.Order_Acks.insert_many(order_acks_to_send.to_dict('records'))
                    print(Fore.BLACK, Back.LIGHTGREEN_EX, f"Sent These Completed Orders To DB At {datetime.datetime.now()}")
                    print(order_acks_to_send)
                    print(Style.RESET_ALL)
                    break
                    
                except Exception as e:
                    print(Fore.RED, f"{e}, Error Inserting Completed Orders Acks Into DB, Trying Again, (tries_left: inf)")
                    print(Style.RESET_ALL)
                    time.sleep(1)
        
        print(Fore.LIGHTMAGENTA_EX, Back.BLACK, f"Order Request Tracker Status After Processing Fills At {datetime.datetime.now()}")
        print(self.order_request_tracker)
        print(Style.RESET_ALL)


    def _process_order_modifications(self, unfilled_orders):
        
        self._get_prices()
        timestamp = datetime.datetime.now().timestamp()
        for order_id, tradingsymbol, absolute_quantity, direction, price, variety, tag, parent_order_id in zip(unfilled_orders.order_id, unfilled_orders.tradingsymbol, unfilled_orders.quantity, unfilled_orders.transaction_type, unfilled_orders.price, unfilled_orders.variety, unfilled_orders.tag, unfilled_orders.parent_order_id):
            
            if direction.upper() == "BUY":
                direction = 1
            else:
                direction = -1

            if order_id in self.unfilled_order_tracker:
                
                if timestamp - self.unfilled_order_tracker[order_id]["last_updated"] >= self.trading_parameters['min_seconds_before_mod'][self.instrument_underlying_map[tradingsymbol]][self.instrument_expiry_map[tradingsymbol]]:
                    
                    #get order modifed count
                    order_modified_count = self.API_Interface.Get_Order_Modified_Count(order_id)
                    
                    #price trail until max mod count and adjusting min price change before modification based on modified count strictly greater than 
                    if order_modified_count > self.trading_parameters['max_mod_count_for_price_trail'][self.instrument_underlying_map[tradingsymbol]][self.instrument_expiry_map[tradingsymbol]] or np.isfinite(parent_order_id):
                        execution_price = self._get_execution_price(tradingsymbol, direction, trail = False)
                        min_price_change_before_mod = self.trading_parameters["min_price_change_before_mod_after_price_trail"][self.instrument_underlying_map[tradingsymbol]][self.instrument_expiry_map[tradingsymbol]]
                    
                    elif order_modified_count == self.trading_parameters['max_mod_count_for_price_trail'][self.instrument_underlying_map[tradingsymbol]][self.instrument_expiry_map[tradingsymbol]]:
                        execution_price = self._get_execution_price(tradingsymbol, direction, trail = False)
                        min_price_change_before_mod = self.trading_parameters["min_price_change_before_mod_until_price_trail"][self.instrument_underlying_map[tradingsymbol]][self.instrument_expiry_map[tradingsymbol]]
                    
                    else:
                        execution_price = self._get_execution_price(tradingsymbol, direction, trail = True)
                        min_price_change_before_mod = self.trading_parameters["min_price_change_before_mod_until_price_trail"][self.instrument_underlying_map[tradingsymbol]][self.instrument_expiry_map[tradingsymbol]]
                    
                    #checking for minimum price change before modificatios
                    if direction * (execution_price / self.unfilled_order_tracker[order_id]["price"] - 1) - min_price_change_before_mod > 1e-8:

                        success_flag = self.API_Interface.Modify_Order(order_id, execution_price, self.unfilled_order_tracker[order_id]["iceberg_flag"])
                        if success_flag is None:
                            continue
                            
                        elif success_flag == -1:
                            print(Fore.RED, f"Manual Order Modification Required For Order {order_id}, {tradingsymbol}, {direction}, {price}, {variety} -> Revised Price {execution_price}")
                            sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
                            user_input = input(Back.LIGHTCYAN_EX + Fore.YELLOW + "Enter Any Character Once Done?")
                            print(Style.RESET_ALL)
                            
                        else:
                            print(Fore.GREEN, f"Succesfully Modified Order For {absolute_quantity * direction} {tradingsymbol} For {tag} From {price} To {execution_price}")
                            print(Style.RESET_ALL)
                            
                        self.unfilled_order_tracker[order_id]["price"] = execution_price
                        self.unfilled_order_tracker[order_id]["last_updated"] = datetime.datetime.now().timestamp()

            else:

                if variety.lower() == "iceberg":
                    iceberg_flag = True
                else:
                    iceberg_flag = False
                    
                self.unfilled_order_tracker[order_id] = {"tradingsymbol" : tradingsymbol,
                                                         "absolute_quantity" : abs(absolute_quantity),
                                                         "direction" : direction,
                                                         "price" : price,
                                                         "last_updated" : datetime.datetime.now().timestamp(),
                                                         "iceberg_flag" : iceberg_flag, 
                                                         "tag" : tag,
                                                         "parent_order_id" : parent_order_id}
    
    def Start(self):
        
        minutes_since_start = (datetime.datetime.now().timestamp() - datetime.datetime.strptime(f"{self.parameters['date_today']}" + " 09:15:00", "%Y-%m-%d %H:%M:%S").timestamp()) / 60
        minutes_since_start_proxy = copy.deepcopy(minutes_since_start)
        kill_all_flag = False
        
        while minutes_since_start<365:
            
            #minute tracker
            minutes_since_start = (datetime.datetime.now().timestamp() - datetime.datetime.strptime(f"{self.parameters['date_today']}" + " 09:15:00", "%Y-%m-%d %H:%M:%S").timestamp()) / 60
            
            self._get_order_request()
            self._send_orders()
            self._process_order_acks()
            
            #printing number of order requests sent
            if minutes_since_start - minutes_since_start_proxy >=5:
                
                minutes_since_start_proxy = copy.deepcopy(minutes_since_start)
                print(Fore.BLACK, Back.LIGHTYELLOW_EX, f"Order Requests Since Start At {datetime.datetime.now()} : {len(self.API_Interface.placed_order_tracker) - 1}")
                print(Style.RESET_ALL)
                
            if not kill_all_flag and minutes_since_start > 345.5:
                
                print(Fore.GREEN, "Sending Kill Alls")
                print(Style.RESET_ALL)
                #switch so this is only done once
                kill_all_flag = True
                
                #get open orders
                order_acks = self.API_Interface.Executions()
                order_acks = order_acks[order_acks.status == "OPEN"]
                for order_id, variety in zip(order_acks.order_id, order_acks.variety):
                    
                    if variety.lower() == "iceberg":
                        iceberg_flag = True
                    else:
                        iceberg_flag = False
                    
                    #cancelling all orders
                    success_flag = self.API_Interface.Cancel_Order(order_id, iceberg_flag)
                
                #check if manual cancellations are required
                sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
                user_input = input(Back.LIGHTCYAN_EX + Fore.GREEN + "Confirm If All Orders Are Cancelled On Kite Web, Press Enter Once Done ?")
                print(Style.RESET_ALL)

                #look for open positions?
                live_positions = self.API_Interface.Live_Positions()
                if len(live_positions) == 0:
                    continue
                
                else:
                    
                    self._get_momentum_indicators()
                    live_positions = live_positions[['tradingsymbol', 'quantity']]
                    live_positions["underlying"] = live_positions.tradingsymbol.map(self.instrument_list.set_index('tradingsymbol').underlying.to_dict())
                    live_positions["strategy_variant"] = 0
                    live_positions["req_number"] = self.order_req_number + 1
                    live_positions["quantity"] *= -1
                    live_positions["priority"] = 0
                    
                    if self.momentum_indicators is not None:
                        
                        live_positions["momentum_indicator"] = live_positions.underlying.map(self.momentum_indicators)
                        live_positions.momentum_indicator.fillna(0, inplace = True)
                        
                        live_positions["instrument_type"] = 0
                        live_positions.instrument_type = np.where(live_positions.tradingsymbol.str.endswith('CE'), 1, live_positions.instrument_type)
                        live_positions.instrument_type = np.where(live_positions.tradingsymbol.str.endswith('PE'), -1, live_positions.instrument_type)
                        
                        live_positions.priority = np.where(live_positions.instrument_type * live_positions.momentum_indicator == 1, 1, live_positions.priority)
                        live_positions.priority = np.where(live_positions.instrument_type * live_positions.momentum_indicator == -1, 0, live_positions.priority)
                        live_positions.drop(columns = ['momentum_indicator', 'instrument_type'], inplace = True)
                        
                    #entering order requests into DB
                    while True:
                        
                        try:
                            client.Live_Trading.Order_Requests.insert_many(live_positions.to_dict('records'))
                            print(Fore.BLACK, Back.LIGHTGREEN_EX, f"Dummy Kill All Orders Sent At {datetime.datetime.now()}")
                            print(live_positions)
                            print(Style.RESET_ALL)
                            break
                            
                        except Exception as e:
                            print(Fore.RED, f"{e}, Could Not Send Kill All Message To Order Requests DB. Trying Again (tries_left: inf)")
                            time.sleep(1)
                    
        print(Fore.GREEN, "Shutting Down Order Central Command. Double Check If All Positions Are 0 Or No Open Orders")
        print(Style.RESET_ALL)

def Get_User_Inputs():
    
    print(Fore.BLUE, Back.LIGHTWHITE_EX, "Incoming Message")
    print(Fore.LIGHTMAGENTA_EX, Back.BLACK, "Class Variables")
    print(Fore.BLACK, Back.LIGHTYELLOW_EX, "Internal Variables")
    print(Fore.BLACK, Back.LIGHTGREEN_EX, "Outgoing Message")
    print(Style.RESET_ALL)

    #default values
    sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
    
    print(Back.LIGHTCYAN_EX + Fore.BLACK)
    parameters = {'underlying' : ['NSE:NIFTY 50'], 
                  'symbol_map' : {"NSE:NIFTY 50" : "NIFTY"},
                  'reversed_symbol_map' : {"NIFTY" : "NSE:NIFTY 50"},
                  'date_today' : input("Today's Date? ").strip()}
    """
    parameters = {'underlying' : ['NSE:NIFTY 50', 'NSE:NIFTY BANK'], 
                  'symbol_map' : {"NSE:NIFTY 50" : "NIFTY", "NSE:NIFTY BANK" : "BANKNIFTY"},
                  'reversed_symbol_map' : {"NIFTY" : "NSE:NIFTY 50", "BANKNIFTY" : "NSE:NIFTY BANK"},
                  'date_today' : input("Today's Date? ").strip()}

    """
    print(Style.RESET_ALL)
    
    trading_parameters = {'paper_trading' : False, 
                          'price_tolerance' : {"NIFTY" : {"current_week" : 0.03, "next_week" : 0.03}}, 
                          'price_trail' : {"NIFTY" : {"current_week" : 0.001, "next_week" : 0.001}}, 
                          'max_mod_count_for_price_trail' : {"NIFTY" : {"current_week" : 0, "next_week" : 0}}, 
                          'min_price_change_before_mod_until_price_trail' : {"NIFTY" : {"current_week" : 0.005, "next_week" : 0.005}}, 
                          'min_price_change_before_mod_after_price_trail' : {"NIFTY" : {"current_week" : 0.005, "next_week" : 0.005}}, 
                          'min_seconds_before_mod' : {"NIFTY" : {"current_week" : 2, "next_week" : 2}}}
    """
    trading_parameters = {'paper_trading' : False, 
                      'price_tolerance' : {"NIFTY" : {"current_week" : 0.02, "next_week" : 0.02}, "BANKNIFTY" : {"current_week" : 0.02, "next_week" : 0.02}}, 
                      'price_trail' : {"NIFTY" : {"current_week" : 0.001, "next_week" : 0.001}, "BANKNIFTY" : {"current_week" : 0.001, "next_week" : 0.001}}, 
                      'max_mod_count_for_price_trail' : {"NIFTY" : {"current_week" : 0, "next_week" : 0}, "BANKNIFTY" : {"current_week" : 0, "next_week" : 0}}, 
                      'min_price_change_before_mod_until_price_trail' : {"NIFTY" : {"current_week" : 0.005, "next_week" : 0.005}, "BANKNIFTY" : {"current_week" : 0.005, "next_week" : 0.005}}, 
                      'min_price_change_before_mod_after_price_trail' : {"NIFTY" : {"current_week" : 0.005, "next_week" : 0.005}, "BANKNIFTY" : {"current_week" : 0.005, "next_week" : 0.005}}, 
                      'min_seconds_before_mod' : {"NIFTY" : {"current_week" : 2, "next_week" : 2}, "BANKNIFTY" : {"current_week" : 2, "next_week" : 2}}}
    """
    #parameters
    print(Fore.LIGHTMAGENTA_EX, Back.BLACK)
    print("parameters")
    for key, value in parameters.items():
        print(f"{key} : {value}, type : {type(value)}")
    print(Style.RESET_ALL)
    
    modified_flag = False
    sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
    print(Back.LIGHTCYAN_EX + Fore.BLACK)
    user_input = input("\nAll Good For :parameters: ? Enter Any Character If Otherwise ")
    print(Style.RESET_ALL)
    
    if len(user_input) !=0 :
        
        modified_flag = True
        for key, value in parameters.items():
            
            sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
            user_input = input(f"\n\nAll Good For :{key} - {value}: ? Enter Any Character If Otherwise ")
            if len(user_input) != 0:

                if key != "date_today":
                    
                    temp = []
                    while True:
                        sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
                        user_input = input(f"\nEnter Value for :{key}: ? Enter Blank To Finalize ").strip().upper()
                        if len(user_input) == 0:
                            break
                        else:
                            temp.append(user_input)

                else:
                    sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
                    temp = input(f"\nEnter Value for :{key}: ? ").strip().upper()

                parameters[key] = temp

    #trading_parameters
    print(Fore.LIGHTMAGENTA_EX, Back.BLACK, f"\ntrading_parameters")
    for key, value in trading_parameters.items():
        print(f"{key} : {value}, type : {type(value)}")
    print(Style.RESET_ALL)
    
    sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
    print(Back.LIGHTCYAN_EX + Fore.BLACK)
    user_input = input("\nAll Good For :trading_parameters: ? Enter Any Character If Otherwise ")
    print(Style.RESET_ALL)
    
    if len(user_input) !=0 :
        
        modified_flag = True
        print('\npaper_trading - whether you want a test or live setup')
        print('\nprice_tolerance - how far away on the unfavourable side(by profitability) would you tolerate price in the market is different from guide price 2% -> 0.02')
        print('\nprice_trail - how far on the favourable side(by profitability) would you nudge the execution price until a maximum modified count is reached 1% -> 0.01')
        print('\nmin_price_change_before_mod_until_price_trail - change price only if execution price is unfavourably(by chance of fill) different than order price by this amount 0.5% -> 0.005')
        print('\nmin_price_change_before_mod_after_price_trail - same as min_price_change_before_mod_until_price_trail but used instead of it after max trail modification')
        print('\nmin_seconds_before_mod - time before we evaluate if an order should be modified')

        for key, value in trading_parameters.items():
            
            sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
            user_input = input(f"\n\nAll Good For :{key} - {value}: ? Enter Any Character If Otherwise ")
            
            if len(user_input) != 0:
                
                sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
                temp = input(f"\nEnter Value for :{key}: ?").strip().upper()
                
                if key == "paper_trading":
                    if temp == "FALSE":
                        trading_parameters[key] = False
                    else:
                        trading_parameters[key] = True

                else:
                    trading_parameters[key] = float(temp)
    
    if modified_flag:
        
        print(Fore.LIGHTMAGENTA_EX, Back.BLACK, f"\nAfter Modification - parameters")
        for key, value in parameters.items():
            print(f"{key} : {value}, type : {type(value)}")

        print("\nAfter Modification - trading_parameters")
        for key, value in trading_parameters.items():
            print(f"{key} : {value}, type : {type(value)}")
        print(Style.RESET_ALL)
        
        sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
        user_input = input("All Okay? Enter Any Character If Otherwise ")
        if len(user_input)!=0:
            raise RuntimeError(Fore.RED + "Start Again !!!")

    return parameters, trading_parameters

parameters, trading_parameters = Get_User_Inputs()
self = Order_Central_Command(parameters, trading_parameters)
self.Start()