from datetime import datetime,date
from math import log, sqrt, pi, exp
import numpy as np
import pandas as pd
import BS
import csv
import os

class Instrument:
    
    def __init__(self, parameters, verbose = False):
        '''
        zerodha map
        name -> underlying
        last_price -> initial_price
        drop segment
        
        parameters = {"portfolio_id":, "strategy_variant":, "initial_quantity":, 
         "initial_underlying_price":, "interest_rate":, "initial_IV":, "initial_delta":, 
        "initial_gamma":, "initial_vega":,
        
        zerodha columns
        "instrument_token": , "exchange_token": ,"tradingsymbol":, "underlying":, "initial_price":,
        "expiry":, "strike":, "tick_size":, "lot_size":, "instrument_type":, "exchange":,}
        '''
        self.verbose = verbose
        
        #initializing variables corresponding to user-defined inputs
        self.portfolio_id = parameters["portfolio_id"]
        self.strategy_variant = parameters["strategy_variant"]
        self.instrument_token = parameters["instrument_token"]
        self.exchange_token = parameters["exchange_token"]
        self.tradingsymbol = parameters["tradingsymbol"]
        self.underlying = parameters["underlying"]
        self.exchange = parameters["exchange"]
        self.instrument_type = parameters["instrument_type"] #[CE,PE,FUT,STK]
        self.expiry = parameters["expiry"] #date variable
        self.strike = parameters["strike"]
        self.lot_size = parameters["lot_size"]
        self.tick_size = parameters["tick_size"]
        self.initial_quantity = parameters["initial_quantity"]
        self.initial_price = parameters["initial_price"]
        self.initial_underlying_price = parameters["initial_underlying_price"]
        self.interest_rate = parameters["interest_rate"]
        self.initial_IV = parameters["initial_IV"]
        self.initial_delta = parameters["initial_delta"]
        self.initial_gamma = parameters["initial_gamma"]
        self.initial_vega = parameters["initial_vega"]

        #initializing other variables        
        self.current_time = datetime.today()
        self.days_to_expiry = self._days_to_expiry()
        self.quantity = parameters["initial_quantity"]
        self.current_price = parameters["initial_price"]
        self.instrument_pnl = 0
        self.delta_pnl = 0        
        self.gamma_pnl = 0
        self.vega_pnl = 0
        self.theta_pnl = 0
        self.current_underlying_price = parameters["initial_underlying_price"]
        self.total_buy_quantity = 0
        self.total_sell_quantity = 0
        self.total_buy_value = 0
        self.total_sell_value = 0
        
        #calculating Greeks
        self.IV = self._set_IV()
        self.delta = self._set_delta()
        self.gamma = self._set_gamma()
        self.vega = self._set_vega()
        self.theta = self._set_theta()
        self.rho = self._set_rho()
        
        #initializing log variables
        self.instrument_log = []
        self.order_log = []
        
        #calling class functions
        self._Instrument_Logger("Initialization")
        if self.verbose:
            print(f"\n{self.tradingsymbol} created for strategy_variant {self.strategy_variant} and portfolio id {self.portfolio_id}")
    
    def __del__(self):

        date_today=self.current_time.strftime("%Y%m%d")
        
        try:
            os.mkdir(f"../{date_today}")
        except:
            if self.verbose:
                print(f"{date_today} Directory Exists")
            
        file_name=date_today+"_"+str(self.tradingsymbol)+"_"+str(self.portfolio_id)+"_"+str(self.strategy_variant)
        
        try:
            order_keys = self.order_log[0].keys()
            with open(f'../{date_today}/Order_{file_name}.csv.', 'w', newline='') as output_file:
                dict_writer = csv.DictWriter(output_file, order_keys)
                dict_writer.writeheader()
                dict_writer.writerows(self.order_log)
        except Exception as e:
            if self.verbose:
                print(f"\nEmpty Order Logs for {self.tradingsymbol} in portfolio {self.portfolio_id} for strategy type {self.strategy_variant}. ",e)
            
        instrument_keys = self.instrument_log[0].keys()
        with open(f'../{date_today}/Instrument_{file_name}.csv.', 'w', newline='') as output_file:
            dict_writer = csv.DictWriter(output_file, instrument_keys)
            dict_writer.writeheader()
            dict_writer.writerows(self.instrument_log)

        if self.verbose:
            print(f"\n{self.tradingsymbol} de-allocated for strategy_variant {self.strategy_variant} and portfolio id {self.portfolio_id}")
        
    def _Instrument_Logger(self, log_type):
        
        try:

            #instrument level logging. This is to be called whenever external user interacts with object methods
            variable_dict={}
            variable_dict["log_type"]=log_type
            variable_dict["portfolio_id"]=self.portfolio_id
            variable_dict["strategy_variant"]=self.strategy_variant
            variable_dict["instrument_token"]=self.instrument_token
            variable_dict["exchange_token"]=self.exchange_token
            variable_dict["tradingsymbol"]=self.tradingsymbol
            variable_dict["underlying"]=self.underlying
            variable_dict["exchange"]=self.exchange
            variable_dict["instrument_type"]=self.instrument_type
            variable_dict["expiry"]=self.expiry
            variable_dict["strike"]=self.strike
            variable_dict["lot_size"]=self.lot_size
            variable_dict["tick_size"]=self.tick_size
            variable_dict["initial_quantity"]=self.initial_quantity
            variable_dict["initial_price"]=self.initial_price
            variable_dict["initial_underlying_price"]=self.initial_underlying_price
            variable_dict["interest_rate"]=self.interest_rate
            variable_dict["initial_IV"]=self.initial_IV
            variable_dict["initial_delta"]=self.initial_delta
            variable_dict["initial_gamma"]=self.initial_gamma
            variable_dict["initial_vega"]=self.initial_vega


            variable_dict["current_time"]=datetime.today()
            variable_dict["days_to_expiry"]=self.days_to_expiry
            variable_dict["quantity"]=self.quantity
            variable_dict["current_price"]=self.current_price
            variable_dict["instrument_pnl"]=self.instrument_pnl
            variable_dict["delta_pnl"]=self.delta_pnl        
            variable_dict["gamma_pnl"]=self.gamma_pnl
            variable_dict["vega_pnl"]=self.vega_pnl
            variable_dict["theta_pnl"]=self.theta_pnl
            variable_dict["current_underlying_price"]=self.current_underlying_price
            variable_dict["total_buy_quantity"]=self.total_buy_quantity
            variable_dict["total_sell_quantity"]=self.total_sell_quantity
            variable_dict["total_buy_value"]=self.total_buy_value
            variable_dict["total_sell_value"]=self.total_sell_value
            
            variable_dict["IV"]=self.IV
            variable_dict["delta"]=self.delta
            variable_dict["gamma"]=self.gamma
            variable_dict["vega"]=self.vega
            variable_dict["theta"]=self.theta
            variable_dict["rho"]=self.rho

            self.instrument_log.append(variable_dict)
            
            if log_type!="Initialization":
                return variable_dict
        
        except Exception as e:
            if self.verbose:
                print(f"\nError in instrument logging for {self.tradingsymbol} in strategy_variant {self.strategy_variant} \
                  and portfolio {self.portfolio_id}. ",e)
            self.instrument_log.append({})
            if log_type!="Initialization":
                return {}
        
    def _Order_Logger(self, orders):
        #list of dictionary variable for order. [{"order_id":,"tradingsymbol":"","quantity":,"price":,"timestamp":}]
        
        try:
            
            for order in orders:

                if all(order_keys in order for order_keys in ["order_id", "tradingsymbol", "quantity", "price", "timestamp"]):
                    if order["tradingsymbol"] == self.tradingsymbol:
                        order["underlying_price"]=self.current_underlying_price
                        order["IV"]=self.IV
                        order["delta"]=self.delta
                        order["gamma"]=self.gamma
                        order["vega"]=self.vega
                        self.order_log.append(order)
                        if self.verbose:
                            print(f"\n\norder log updated for {self.tradingsymbol} in portfolio {self.portfolio_id} -> {order}")
                    else:
                        if self.verbose:
                            print("\nWrong identifier used. {} transaction used to update {} in strategy_variant \
                              {} and in portfolio {}".format(order["tradingsymbol"],self.tradingsymbol,self.strategy_variant,self.portfolio_id))

                else:
                    if self.verbose:
                        print(f"\nOrder dictionary not passed in the required format for {self.tradingsymbol} in strategy_variant \
                          {self.strategy_variant} and portfolio {self.portfolio_id}. {order}")

        except Exception as e:
            if self.verbose:
                print(f"\nError in order logging for {self.tradingsymbol} in strategy_variant {self.strategy_variant} and portfolio {self.portfolio_id}. ",e)
            
    def Log_Retriever(self, log_type):
         
        try:
            if log_type=="Order":
                return self.order_log
            elif log_type=="Instrument":
                return self.instrument_log
            else:
                return []
        except Exception as e:
            if self.verbose:
                print(f"\nCould not retrieve logs for  {self.tradingsymbol} in strategy_variant {self.strategy_variant} and portfolio {self.portfolio_id}. ",e)
            return []
            
    def Calculate_Pnl(self, price, underlying_price, direct_call, greek_update):
        
        try:
            
            self.current_price=price
            self.current_underlying_price=underlying_price

            try:
                buy_average=self.total_buy_value/self.total_buy_quantity
            except:
                buy_average=price

            try:
                sell_average=self.total_sell_value/self.total_sell_quantity
            except:
                sell_average=price

            self.instrument_pnl=self.initial_quantity*(price-self.initial_price) + self.total_sell_quantity*(sell_average-price) + \
            self.total_buy_quantity*(price-buy_average)        

            if greek_update == True:
                self.Update_Greeks(price,underlying_price,False)

            if direct_call==True:
                return self._Instrument_Logger("Price_Update")
        
        except Exception as e:
            if self.verbose:
                print(f"\nError in calculating PnL for {self.tradingsymbol} in strategy_variant {self.strategy_variant} and portfolio {self.portfolio_id}. ",e)

        
    def Update_Order(self, orders, price, underlying_price, greek_update):
        #list of dictionary variable for order. [{"order_id":,"tradingsymbol":"","quantity":,"price":,"timestamp":}]

        try:
            for order in orders:

                if order["tradingsymbol"] == self.tradingsymbol and all(order_keys in order for order_keys in \
                                                      ["order_id","tradingsymbol","quantity","price","timestamp"]):
                    self.quantity+=order["quantity"]

                    if order["quantity"]>0:
                        self.total_buy_quantity+=order["quantity"]
                        self.total_buy_value+=order["quantity"]*order["price"]
                    else:
                        self.total_sell_quantity-=order["quantity"]
                        self.total_sell_value-=order["quantity"]*order["price"]
                else:
                    raise RuntimeError("Improper ", self.portfolio_id)
            
            self.Calculate_Pnl(price,underlying_price,False,greek_update)
            self._Order_Logger(orders)

            return self._Instrument_Logger("Order_Update")
        
        except Exception as e:
            if self.verbose:
                print(f"\nError in updating order for {self.tradingsymbol} in strategy_variant {self.strategy_variant} and portfolio {self.portfolio_id}. ",e)
            raise RuntimeError("Error updating_order_at instrument_level", (self.strategy_variant, self.portfolio_id))

    def _days_to_expiry(self):
        
        self.current_time=datetime.today()
        days_to_expiry=np.busday_count(date.today(),self.expiry) + 1
        day_end = datetime(date.today().year, date.today().month, date.today().day, 15, 30, 0, 0)
        
        if (day_end - self.current_time).days < 0:
            difference=1
        else:
            difference = (22500-(day_end - self.current_time).seconds)/22500
        
        if difference>1:
            difference=0
        
        return max(days_to_expiry-difference,0.01)
            
    def _set_IV(self):
        
        try:
            
            if self.instrument_type=="CE":
                return BS.call_implied_volatility(self.current_price,self.current_underlying_price, \
                                                  self.strike,self.days_to_expiry/252,self.interest_rate)
            elif self.instrument_type=="PE":
                return BS.put_implied_volatility(self.current_price,self.current_underlying_price, \
                                                 self.strike,self.days_to_expiry/252,self.interest_rate)
            else:
                return 0
        
        except Exception as e:
            if self.verbose:
                print(f"\nCould not calculate IV for {self.tradingsymbol} in strategy_variant {self.strategy_variant} and portfolio {self.portfolio_id}. ",e)
            return 0.15
            
    def _set_delta(self):

        try:
            
            if self.instrument_type=="CE":
                return BS.call_delta(self.current_underlying_price,self.strike,self.days_to_expiry/252,self.interest_rate,self.IV)
            elif self.instrument_type=="PE":
                return BS.put_delta(self.current_underlying_price,self.strike,self.days_to_expiry/252,self.interest_rate,self.IV)
            elif self.instrument_type=="FUT":
                return exp(self.interest_rate*self.days_to_expiry/252)
            else:
                return 1
        
        except Exception as e:
            if self.verbose:
                print(f"\nCould not calculate delta for {self.tradingsymbol} in strategy_variant {self.strategy_variant} \
                  and portfolio {self.portfolio_id}. ",e)
            return 0.5
        
    def _set_gamma(self):
        
        try:
            
            if self.instrument_type=="CE":
                return BS.call_gamma(self.current_underlying_price,self.strike,self.days_to_expiry/252,self.interest_rate,self.IV)
            elif self.instrument_type=="PE":
                return BS.put_gamma(self.current_underlying_price,self.strike,self.days_to_expiry/252,self.interest_rate,self.IV)
            else:
                return 0
        
        except Exception as e:
            if self.verbose:
                print(f"\nCould not calculate gamma for {self.tradingsymbol} in strategy_variant {self.strategy_variant} \
                  and portfolio {self.portfolio_id}. ",e)
            return 0

    def _set_vega(self):
        
        try:

            if self.instrument_type=="CE":
                return BS.call_vega(self.current_underlying_price,self.strike,self.days_to_expiry/252,self.interest_rate,self.IV)
            elif self.instrument_type=="PE":
                return BS.put_vega(self.current_underlying_price,self.strike,self.days_to_expiry/252,self.interest_rate,self.IV)
            else:
                return 0
        
        except Exception as e:
            if self.verbose:
                print(f"\nCould not calculate vega for {self.tradingsymbol} in strategy_variant {self.strategy_variant} and portfolio {self.portfolio_id}. ",e)
            return 0


    def _set_theta(self):
        
        try:
            if self.instrument_type=="CE":
                return BS.call_theta(self.current_underlying_price,self.strike,self.days_to_expiry/252,self.interest_rate,self.IV)
            elif self.instrument_type=="PE":
                return BS.put_theta(self.current_underlying_price,self.strike,self.days_to_expiry/252,self.interest_rate,self.IV)
            elif self.instrument_type=="FUT":
                return self.interest_rate*self.current_underlying_price*exp(self.interest_rate*self.days_to_expiry/252)
            else:
                return 0
        
        except Exception as e:
            if self.verbose:
                print(f"\nCould not calculate theta for {self.tradingsymbol} in strategy_variant {self.strategy_variant} \
                  and portfolio {self.portfolio_id}. ",e)
            return 0

    def _set_rho(self):
        
        try:
            if self.instrument_type=="CE":
                return BS.call_rho(self.current_underlying_price,self.strike,self.days_to_expiry/252,self.interest_rate,self.IV)
            elif self.instrument_type=="PE":
                return BS.put_rho(self.current_underlying_price,self.strike,self.days_to_expiry/252,self.interest_rate,self.IV)
            elif self.instrument_type=="FUT":
                return (self.days_to_expiry/252)*self.current_underlying_price*exp(self.interest_rate*self.days_to_expiry/252)
            else:
                return 0
        
        except Exception as e:
            if self.verbose:
                print(f"\nCould not calculate rho for {self.tradingsymbol} in strategy_variant {self.strategy_variant} \
                  and portfolio {self.portfolio_id}. ",e)
            return 0
    
    def Update_Greeks(self,price,underlying_price,direct_call):
        
        try:
            
            self.days_to_expiry=self._days_to_expiry()
            self.current_price=price
            self.current_underlying_price=underlying_price

            self.IV=self._set_IV()
            self.delta=self._set_delta()
            self.gamma=self._set_gamma()
            self.vega=self._set_vega()
            self.theta=self._set_theta()
            self.rho=self._set_rho()

            order_history = pd.DataFrame(self.Log_Retriever("Order"))

            if len(order_history) == 0:

                self.delta_pnl=0            
                self.gamma_pnl=0            
                self.vega_pnl=0
                self.theta_pnl=0

            else:
                order_history["current_underlying_price"]=self.current_underlying_price
                order_history["current_IV"]=self.IV

                order_history["delta_pnl"]=(order_history.quantity)* \
                (order_history.current_underlying_price-order_history.underlying_price)*(order_history.delta)
                order_history["gamma_pnl"]=(order_history.quantity)*((order_history.current_underlying_price- \
                                                                      order_history.underlying_price)**2)*order_history.gamma*0.5
                order_history["vega_pnl"]=(order_history.quantity)*(order_history.current_IV-order_history.IV)*(order_history.vega)

                self.delta_pnl=order_history.delta_pnl.sum()
                self.vega_pnl=order_history.vega_pnl.sum()
                self.gamma_pnl=order_history.gamma_pnl.sum()

            self.delta_pnl+=(self.initial_quantity)*(self.current_underlying_price-self.initial_underlying_price)*(self.initial_delta)
            self.gamma_pnl+=(self.initial_quantity)*((self.current_underlying_price- \
                                                      self.initial_underlying_price)**2)*self.initial_gamma*0.5
            self.vega_pnl+=(self.initial_quantity)*(self.IV-self.initial_IV)*(self.initial_vega)
            self.theta_pnl=self.instrument_pnl-self.delta_pnl-self.gamma_pnl-self.vega_pnl

        except Exception as e:
            if self.verbose:
                print(f"\nCould not calculate greek pnl for {self.tradingsymbol} in strategy_variant {self.strategy_variant} and portfolio {self.portfolio_id}. ",e)
        
        if direct_call ==True:
            return self._Instrument_Logger("Greek_Update")
