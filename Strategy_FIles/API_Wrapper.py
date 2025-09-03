from colorama import Fore, Back, Style
import pandas as pd
import numpy as np
import Config
import time
import os
import datetime
import warnings
warnings.filterwarnings("ignore")
from kiteconnect import KiteConnect
import webbrowser

def segregate_tag(tag):
    
    pos = tag.find(':')
    if pos>0:
        tag = tag[pos+1:]
        pos = tag.find(':')
        if pos>0:
            tag = tag[:pos]
    return tag.upper()

class API_Wrapper:
    
    def __init__(self, market_adapter = True, paper_trading = True):
        
        #init variables
        self.paper_trading = paper_trading
        self.kite = None
        self.active_status = True
        self.quote_request_tracker = [datetime.datetime.now().timestamp()]
        self.placed_order_tracker = [datetime.datetime.now().timestamp()]
        self.general_request_tracker = [datetime.datetime.now().timestamp()]
        self.modified_order_count = {} #{"<order_id>": 0}
        self.init_time = datetime.datetime.now()
        
        if market_adapter:
            api_access_card = Config.kite_market_data_adapter
        else:
            api_access_card = Config.kite_order_manager

        api_key = api_access_card["api_key"]
        api_secret = api_access_card["api_secret"]
        kite = KiteConnect(api_key = api_key)
        tries = 2
        
        while tries > 0:
            
            try:
                print(Fore.GREEN + f"Get Request Token From https://kite.trade/connect/login?api_key={api_key}")
                print(Style.RESET_ALL)
                webbrowser.open(f"https://kite.trade/connect/login?api_key={api_key}")
                sound = os.system(f'afplay {Config.parent_directory}/Temp_Storage/notification.mp3')
                request_token = input(Back.LIGHTCYAN_EX + Fore.BLACK + "Enter Request Token: ")
                print(Style.RESET_ALL)

                kite_session = kite.generate_session(request_token, api_secret = api_secret)
                kite.set_access_token(kite_session["access_token"])

                if len(kite.instruments(exchange="NFO")) != 0:
                    self.kite = kite
                    break
                    
                else:
                    raise RuntimeError(Fore.RED + f"No Response From API_Key {api_key}")
                    print(Style.RESET_ALL)
                
            except Exception as e:
                tries -= 1
                if tries == 0:
                    raise RuntimeError(Fore.RED + f"{e}, {api_key} Not Initialized;")
                    print(Style.RESET_ALL)
                else:
                    print(Fore.RED, f"{e}, Error!! Try Again\n")
                    print(Style.RESET_ALL)
        
        print(Fore.GREEN, f"{api_key} API initiated")
        print(Style.RESET_ALL)

    def _process_general_wait_time(self):

        while True:

            timestamp_now = datetime.datetime.now().timestamp()
            rolling_requests = [ timestamp for timestamp in self.general_request_tracker if timestamp_now - timestamp <= 1]

            if len(rolling_requests) >= Config.kite_other_requests_per_sec:
                time.sleep(Config.kite_halt_time)

            else:
                self.general_request_tracker = rolling_requests
                break
        
        self.general_request_tracker.append(datetime.datetime.now().timestamp())
    
    def Instruments(self, exchange = "NFO"):
        
        self._process_general_wait_time()
        
        try:
            
            instrument_list = pd.DataFrame(self.kite.instruments(exchange = exchange))
            instrument_list.drop(columns = ['segment', 'last_price'], inplace = True)
            instrument_list.rename(columns = {'name': 'underlying'}, inplace = True)
            instrument_list.instrument_token = instrument_list.instrument_token.astype(int)
            instrument_list.exchange_token = instrument_list.exchange_token.astype(int)
            instrument_list.tradingsymbol = instrument_list.tradingsymbol.astype(str).str.strip()
            instrument_list.underlying = instrument_list.underlying.astype(str).str.strip()
            instrument_list.expiry = instrument_list.expiry.astype(str).str.strip()
            instrument_list.strike = instrument_list.strike.astype(float)
            instrument_list.tick_size = instrument_list.tick_size.astype(float)
            instrument_list.lot_size = instrument_list.lot_size.astype(int)
            instrument_list.instrument_type = instrument_list.instrument_type.astype(str).str.strip()
            instrument_list.exchange = instrument_list.exchange.astype(str).str.strip()
            
            if len(instrument_list) == 0:
                raise RuntimeError("Blank Instrument List Received")
                
            return instrument_list
        
        except Exception as e:
            raise RuntimeError(Fore.RED, f"{e}, Unable To Get Instrument List")
            print(Style.RESET_ALL)

    def Quote(self, instruments):
        
        try:
            while True:
                
                timestamp_now = datetime.datetime.now().timestamp()
                rolling_requests = [ timestamp for timestamp in self.quote_request_tracker if timestamp_now - timestamp <= 1]
                
                if len(rolling_requests) >= Config.kite_quote_per_sec:
                    time.sleep(Config.kite_halt_time)
                    
                else:
                    self.quote_request_tracker = rolling_requests
                    break
            
            self.quote_request_tracker.append(datetime.datetime.now().timestamp())
            return self.kite.quote(instruments)
        
        except Exception as e:
            raise RuntimeError(f"{e}, Unable To Get Market Quotes;")
    
    def Place_Order(self, order):
        
        #order = {"tradingsymbol":,"quantity": ,"tag": , "price": ,"iceberg_legs": ,"iceberg_quantity": }
        if not self.active_status:
            print(Fore.RED, "Cannot Place Any More Orders As Limit May Have Been Reached. Terminate Everything!!!")
            print(Style.RESET_ALL)
            return -1
        
        while True:
            
            timestamp_now = datetime.datetime.now().timestamp()
            time_since_last_request = np.array([ timestamp_now - timestamp for timestamp in self.placed_order_tracker])
            
            halt_flag = False
            if len(time_since_last_request[time_since_last_request <= 1]) >= Config.kite_order_per_sec:
                halt_flag = True
                
            elif len(time_since_last_request[time_since_last_request <= 60]) >= Config.kite_order_per_min:
                halt_flag = True
                
            elif len(time_since_last_request) >= Config.kite_order_per_day * 0.97:
                raise RuntimeError(Fore.RED, f"Cannot Place Any More Orders As Limit May Have Been Reached")
            
            if halt_flag:
                time.sleep(Config.kite_halt_time)
                continue
            else:
                break
            
        iceberg_legs = order["iceberg_legs"]
        iceberg_quantity = order["iceberg_quantity"]
        if iceberg_legs is None:
            variety = self.kite.VARIETY_REGULAR
        else:
            variety = self.kite.VARIETY_ICEBERG

        quantity = abs(order["quantity"])
        if order["quantity"] >=0:
            transaction_type = self.kite.TRANSACTION_TYPE_BUY
        else:
            transaction_type = self.kite.TRANSACTION_TYPE_SELL

        price = order["price"]
        exchange = self.kite.EXCHANGE_NFO
        tradingsymbol = order["tradingsymbol"]
        product = self.kite.PRODUCT_NRML
        order_type = self.kite.ORDER_TYPE_LIMIT
        tag = order["tag"]
        
        try:
            if self.paper_trading:
                return 1

            order_id = int(self.kite.place_order(variety = variety, exchange = exchange, tradingsymbol = tradingsymbol, transaction_type = transaction_type, quantity = quantity, product = product, order_type = order_type, price = price, iceberg_legs = iceberg_legs, iceberg_quantity = iceberg_quantity, tag = tag))            
            self.placed_order_tracker.append(datetime.datetime.now().timestamp())
            return order_id

        except Exception as e:
            print(Fore.RED, f"{e}, Could Not Place Order. {order}")
            print(Style.RESET_ALL)
            return -1
        
    def Modify_Order(self, order_id, price, iceberg_flag):
        
        self._process_general_wait_time()
        if order_id not in self.modified_order_count:
            self.modified_order_count[order_id] = 0
        elif self.modified_order_count[order_id] >= Config.kite_max_modification_limit:
            return -1
        
        try:
            if iceberg_flag:
                variety = self.kite.VARIETY_ICEBERG
            else:
                variety = self.kite.VARIETY_REGULAR

            if self.paper_trading:
                return 1

            order_id_temp = int(self.kite.modify_order(variety = variety, order_id = order_id, price = price))
            self.modified_order_count[order_id] += 1
            return order_id

        except Exception as e:
            print(Fore.RED, f"{e}, Could Not Modify Order With Id {order_id}, Price {price} & Iceberg Flag {iceberg_flag}")
            print(Style.RESET_ALL)
            return None
        
    def Cancel_Order(self, order_id, iceberg_flag):
        
        self._process_general_wait_time()
        try:
            if iceberg_flag:
                variety = self.kite.VARIETY_ICEBERG
            else:
                variety = self.kite.VARIETY_REGULAR
            
            if self.paper_trading:
                return 1

            order_id = int(self.kite.cancel_order(variety = variety, order_id = order_id))
            return order_id
                           
        except Exception as e:
            print(Fore.RED, f"{e}, Could Not Cancel Order With Id {order_id}")
            print(Style.RESET_ALL)
            return -1
    
    def Live_Positions(self):
        
        self._process_general_wait_time()
        try:
            positions =  self.kite.positions()
            positions = pd.DataFrame(positions['net'])
            
            if len(positions) != 0:
                positions = positions[(positions.quantity != 0) & (positions.exchange == 'NFO')]
                positions.tradingsymbol = positions.tradingsymbol.astype(str).str.strip()
                positions.quantity = positions.quantity.astype(int)
            
            return positions
        
        except Exception as e:
            print(Fore.RED, f"{e}, Could Not Retrieve Live Positions")
            print(Style.RESET_ALL)
            return None

    def Executions(self):
        
        self._process_general_wait_time()
        try:
            executions =  pd.DataFrame(self.kite.orders())
            if len(executions) !=0:
                executions = executions[~executions.tag.isna()]
                executions.tag = executions.tag.apply(lambda x: segregate_tag(x))
                executions = executions[executions.status.isin(['COMPLETE', 'OPEN'])]
                
                #type correction if required
                executions.order_id = executions.order_id.astype(int)
                executions.parent_order_id = executions.parent_order_id.astype(float)
                executions.status = executions.status.astype(str).str.strip()
                executions.variety = executions.variety.astype(str).str.strip()
                executions.tradingsymbol = executions.tradingsymbol.astype(str).str.strip()
                executions.quantity = executions.quantity.astype(int)
                executions.average_price = executions.average_price.astype(float)
                executions.tag = executions.tag.astype(str).str.strip()
                executions.transaction_type = executions.transaction_type.astype(str).str.strip()
                executions.price = executions.price.astype(float)
                executions = executions[executions.order_timestamp>=self.init_time]
                
            return executions
        
        except Exception as e:
            print(Fore.RED, f"{e}, Could Not Retrieve Order Executions")
            print(Style.RESET_ALL)
            return None
    
    def Get_Order_Modified_Count(self, order_id):
        
        if order_id in self.modified_order_count:
            return self.modified_order_count[order_id]
        else:
            return 0