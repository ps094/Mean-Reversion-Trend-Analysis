DB_Hostname = "localhost"
DB_Port = 27017

Data_DB = "Strategy"
NIFTYOHLC_COLLECTION = "NIFTYOHLC"

Live_DB = "Live_Trading"
Order_Requests_Collection = "Order_Requests"

interest_rate = 0.07
lot_size = {"NIFTY": 50, "BANKNIFTY": 50}
tick_size = 0.05
qty_limit_per_order = {"NIFTY":1800, "BANKNIFTY":1200}

parent_directory = '/Users/praneetshaw/Desktop/Strategy/'

#kite specific
kite_order_manager = {"api_key": '6x8qv44f5ujzxjd0', "api_secret": 'wkut36radrfk28vr2to4wmqhpv67akxv'}
kite_market_data_adapter = {"api_key": '6x8qv44f5ujzxjd0', "api_secret": 'vxm9p1fygrqtaf850wnq30gr0ah0ggs8'}
kite_halt_time = 0.1
kite_iceberg_leg_limit = 10
kite_quote_per_sec = 1
kite_other_requests_per_sec = 10
#order based limits
kite_max_modification_limit = 20
kite_order_per_sec = 10
kite_order_per_min = 200
kite_order_per_day = 3000
