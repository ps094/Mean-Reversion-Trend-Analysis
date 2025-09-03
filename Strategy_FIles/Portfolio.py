from datetime import datetime,date
import Instrument
import csv

class Portfolio:
    
    def __init__(self, portfolio_id, strategy_variant, instrument_list, interest_rate):
        #parameters = { 
        #zerodha columns
        #"instrument_token": , "exchange_token": ,"tradingsymbol":,
        
        #instrument_list=[{"name":"","exchange":"","instrument_type":"","underlying":"","expiry":"","strike":,"lot_size":,
        #"tick_size":"initial_quantity":,"initial_price":,"initial_underlying_price":,"initial_IV":,"initial_delta":,
        #"initial_gamma":,"initial_vega":},{},{}]
        
        self.portfolio_id=portfolio_id
        self.strategy_variant=strategy_variant
        self.interest_rate=interest_rate
        self.instrument_objects=[]
        self.instrument_logs=[]
        self.instrument_name=[]
        self.underlying_name=[]
        self.portfolio_logs=[]
        
        self.portfolio_delta={}
        self.portfolio_gamma={}
        self.portfolio_vega={}
        self.portfolio_theta=0

        self.portfolio_pnl=0
        self.portfolio_delta_pnl={}
        self.portfolio_gamma_pnl={}
        self.portfolio_vega_pnl={}
        self.portfolio_theta_pnl=0
        
        self.instrument_wise_pnl={}
        self.instrument_wise_delta_pnl={}
        self.instrument_wise_gamma_pnl={}
        self.instrument_wise_vega_pnl={}
        self.instrument_wise_theta_pnl={}

        print(f"\nPortfolio with id {self.portfolio_id} created for strategy variant {self.strategy_variant} at {datetime.now()}")
        self.Add_Instruments(instrument_list)

    def __del__(self):

        date_today=datetime.today().strftime("%Y%m%d")
        try:
            os.mkdir(f"../{date_today}")
        except:
            pass
            #print(f"{date_today} Directory Exists")
                        
        del self.instrument_objects[:]
        portfolio_keys = self.portfolio_logs[0].keys()
        file_name=date_today+"_"+str(self.portfolio_id)+"_"+str(self.strategy_variant)

        with open(f'../{date_today}/Portfolio_{file_name}.csv', 'w', newline='') as output_file:
            dict_writer = csv.DictWriter(output_file, portfolio_keys)
            dict_writer.writeheader()
            dict_writer.writerows(self.portfolio_logs)
        
    def _Update_Metrics(self):
        
        self.portfolio_delta = dict.fromkeys(self.portfolio_delta, 0)
        self.portfolio_gamma = dict.fromkeys(self.portfolio_gamma, 0)
        self.portfolio_vega = dict.fromkeys(self.portfolio_vega, 0)
        self.portfolio_theta = 0

        self.portfolio_pnl = 0
        self.portfolio_delta_pnl = dict.fromkeys(self.portfolio_delta_pnl, 0)
        self.portfolio_gamma_pnl = dict.fromkeys(self.portfolio_gamma_pnl, 0)
        self.portfolio_vega_pnl = dict.fromkeys(self.portfolio_vega_pnl, 0)
        self.portfolio_theta_pnl = 0
        
        self.instrument_wise_pnl = dict.fromkeys(self.instrument_wise_pnl, 0)
        self.instrument_wise_delta_pnl = dict.fromkeys(self.instrument_wise_delta_pnl, 0)
        self.instrument_wise_gamma_pnl = dict.fromkeys(self.instrument_wise_gamma_pnl, 0)
        self.instrument_wise_vega_pnl = dict.fromkeys(self.instrument_wise_vega_pnl, 0)
        self.instrument_wise_theta_pnl = dict.fromkeys(self.instrument_wise_theta_pnl, 0)

        for index, i in enumerate(self.instrument_name):
                      
            log=self.instrument_logs[index]
            
            if log["underlying"] in self.portfolio_delta:
                
                self.portfolio_delta[log["underlying"]] += log["delta"]*log["quantity"]
                self.portfolio_gamma[log["underlying"]] += log["gamma"]*log["quantity"]
                self.portfolio_vega[log["underlying"]] += log["vega"]*log["quantity"]
                self.portfolio_theta += log["theta"]*log["quantity"]
                
                self.portfolio_pnl += log["instrument_pnl"]
                self.portfolio_delta_pnl[log["underlying"]] += log["delta_pnl"]
                self.portfolio_gamma_pnl[log["underlying"]] += log["gamma_pnl"]
                self.portfolio_vega_pnl[log["underlying"]] += log["vega_pnl"]
                self.portfolio_theta_pnl += log["theta_pnl"]
                
            else:
                
                self.portfolio_delta[log["underlying"]] = log["delta"]*log["quantity"]
                self.portfolio_gamma[log["underlying"]] = log["gamma"]*log["quantity"]
                self.portfolio_vega[log["underlying"]] = log["vega"]*log["quantity"]
                self.portfolio_theta += log["theta"]*log["quantity"]

                self.portfolio_pnl += log["instrument_pnl"]
                self.portfolio_delta_pnl[log["underlying"]] = log["delta_pnl"]
                self.portfolio_gamma_pnl[log["underlying"]] = log["gamma_pnl"]
                self.portfolio_vega_pnl[log["underlying"]] = log["vega_pnl"]
                self.portfolio_theta_pnl += log["theta_pnl"]
            
            if log["tradingsymbol"] in self.instrument_wise_pnl:
                
                self.instrument_wise_pnl[log["tradingsymbol"]] += log["instrument_pnl"]
                self.instrument_wise_delta_pnl[log["tradingsymbol"]] += log["delta_pnl"]
                self.instrument_wise_gamma_pnl[log["tradingsymbol"]] += log["gamma_pnl"]                
                self.instrument_wise_vega_pnl[log["tradingsymbol"]] += log["vega_pnl"]
                self.instrument_wise_theta_pnl[log["tradingsymbol"]] += log["theta_pnl"]
                            
            else:
                
                self.instrument_wise_pnl[log["tradingsymbol"]] = log["instrument_pnl"]
                self.instrument_wise_delta_pnl[log["tradingsymbol"]] = log["delta_pnl"]
                self.instrument_wise_gamma_pnl[log["tradingsymbol"]] = log["gamma_pnl"]                
                self.instrument_wise_vega_pnl[log["tradingsymbol"]] = log["vega_pnl"]
                self.instrument_wise_theta_pnl[log["tradingsymbol"]] = log["theta_pnl"]
                
    def _Portfolio_Logger(self,log_type,return_flag):
        
        self._Update_Metrics()
        
        variable_dict={}
        variable_dict["log_type"]=log_type
        variable_dict["portfolio_id"]=self.portfolio_id
        variable_dict["strategy_variant"]=self.strategy_variant
        variable_dict["interest_rate"]=self.interest_rate
        variable_dict["instrument_name"]=self.instrument_name
        variable_dict["underlying_name"]=self.underlying_name
        variable_dict["current_time"]=datetime.now()
        
        variable_dict["portfolio_delta"]=self.portfolio_delta
        variable_dict["portfolio_gamma"]=self.portfolio_gamma
        variable_dict["portfolio_vega"]=self.portfolio_vega
        variable_dict["portfolio_theta"]=self.portfolio_theta

        variable_dict["portfolio_pnl"]=self.portfolio_pnl
        variable_dict["portfolio_delta_pnl"]=self.portfolio_delta_pnl
        variable_dict["portfolio_gamma_pnl"]=self.portfolio_gamma_pnl
        variable_dict["portfolio_vega_pnl"]=self.portfolio_vega_pnl
        variable_dict["portfolio_theta_pnl"]=self.portfolio_theta_pnl
        
        variable_dict["instrument_wise_pnl"]=self.instrument_wise_pnl
        variable_dict["instrument_wise_delta_pnl"]=self.instrument_wise_delta_pnl
        variable_dict["instrument_wise_gamma_pnl"]=self.instrument_wise_gamma_pnl
        variable_dict["instrument_wise_vega_pnl"]=self.instrument_wise_vega_pnl
        variable_dict["instrument_wise_theta_pnl"]=self.instrument_wise_theta_pnl
        
        self.portfolio_logs.append(variable_dict)
        
        if return_flag == True:
            return variable_dict
    
    def Log_Retriever(self,portfolio_flag,instruments,log_types):
        
        if portfolio_flag == True:
            
            return self.portfolio_logs
        
        else:
            
            instrument_log=[]
            for n, i in enumerate(instruments):
                
                try:
                    index=self.instrument_name.index(i)
                    instrument_log.append((self.instrument_objects[index]).Log_Retriever(log_types[n]))
                except:
                    continue
            return instrument_log
        
        
    def Add_Instruments(self,instrument_list):
        
        for index, i in enumerate(instrument_list):
            
            if all(instrument_keys in i for instrument_keys in ["tradingsymbol","exchange","instrument_type","underlying", "expiry", \
                                                                "strike","lot_size","tick_size", "initial_quantity", \
                                                                "initial_price", "initial_underlying_price","initial_IV", \
                                                                "initial_delta","initial_gamma","initial_vega"]):
                if i["tradingsymbol"] not in self.instrument_name:
                    
                    try:
                        
                        params = {"portfolio_id": self.portfolio_id, "strategy_variant": self.strategy_variant,
                                  "initial_quantity": i["initial_quantity"], "initial_underlying_price": i["initial_underlying_price"],
                                  "interest_rate": self.interest_rate, "initial_IV": i["initial_IV"],
                                  "initial_delta": i["initial_delta"], "initial_gamma": i["initial_gamma"],
                                  "initial_vega": i["initial_vega"], "instrument_token": i["instrument_token"],
                                  "exchange_token": i["exchange_token"], "tradingsymbol": i["tradingsymbol"],
                                  "underlying": i["underlying"], "initial_price": i["initial_price"],
                                  "expiry": i["expiry"], "strike": i["strike"], "tick_size": i["tick_size"],
                                  "lot_size": i["lot_size"], "exchange": i["exchange"], 
                                  "instrument_type": i["instrument_type"]}
                        self.instrument_objects.append(Instrument.Instrument(params))
                        self.instrument_name.append(i["tradingsymbol"])
                        self.underlying_name.append(i["underlying"])
                        self.instrument_logs.append((self.instrument_objects[index].Log_Retriever("Instrument"))[0])

                    except Exception as e:
                        raise RuntimeError("Bad instrument_datatype_for instrument_addition", self.portfolio_id)
                        
                else:
                    print("{} is already added.".format(i["tradingsymbol"]))
                    
            else:
                raise RuntimeError("Unmatched portfolio_keys_for instrument_addition", self.portfolio_id)
    
        self._Portfolio_Logger("Instrument_Addition", False)
    
    def _Group_Orders(self,orders):

        Grouped_Orders=[]
        first_run=True

        for i in self.instrument_name:
            
            temp=[]

            try:
                for order in orders:
                    
                    order["timestamp"] = datetime.today()
                    if all(order_keys in order for order_keys in ["order_id","tradingsymbol","quantity","price","timestamp"]):
                        if order["tradingsymbol"] in self.instrument_name:
                            if order["tradingsymbol"]==i:
                                temp.append(order)
                        else:
                            if first_run==True:
                                print("\n{} transaction used to update {} in strategy_variant {} and in portfolio {}. \
                                      Not Updating: ".format(order["tradingsymbol"],i,self.strategy_variant,self.portfolio_id))
                    else:
                        raise RuntimeError("Unmatched order_keys_for order_update", (self.strategy_variant, self.portfolio_id))
            
            except Exception as e:
                raise RuntimeError("Order Grouping Error", (self.strategy_variant, self.portfolio_id))
                
            Grouped_Orders.append(temp)
            first_run=False
        
        return Grouped_Orders
                
        
    def Update_Order(self, orders, price_dict, greek_update):
        #price_dict={"100CE":23,"100CE":3, "NIFTY":16000,"BANKNIFTY",34000}
        #Greek_Update -> True/False
        #Orders=[{"order_id":,"tradingsymbol":"","quantity":,"price":,"timestamp":},{},{}]

        Grouped_Orders=self._Group_Orders(orders)
        
        for index, i in enumerate(self.instrument_objects):

            if len(Grouped_Orders[index]) > 0:    

                try:
                    _underlying_price_temp=price_dict[self.underlying_name[index]]
                except:
                    _underlying_price_temp=i.current_underlying_price

                try:
                    _price_temp=price_dict[self.instrument_name[index]]
                except:
                    _price_temp=i.current_price

                self.instrument_logs[index] = i.Update_Order(Grouped_Orders[index], _price_temp, _underlying_price_temp, greek_update)
        
        return self._Portfolio_Logger("Order_Update", True)
    
    def Calculate_Pnl(self, price_dict, greek_update):
        #price_dict={"100CE":23,"100CE":3, "NIFTY":16000,"BANKNIFTY",34000}
        #Greek_Update -> True/False
        
        #price,underlying_price,direct_call,greek_update

        for index, i in enumerate(self.instrument_objects):

            try:
                _underlying_price_temp=price_dict[self.underlying_name[index]]
            except:
                _underlying_price_temp=i.current_underlying_price

            try:
                _price_temp=price_dict[self.instrument_name[index]]
            except:
                _price_temp=i.current_price

            self.instrument_logs[index] = i.Calculate_Pnl(_price_temp, _underlying_price_temp, True, greek_update)
        
        return self._Portfolio_Logger("Price_Update",True)