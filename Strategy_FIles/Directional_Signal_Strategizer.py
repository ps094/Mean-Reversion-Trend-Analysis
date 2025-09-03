import Directional_Signal_Generator
import Config
import Portfolio
import datetime
import pickle
import os

from colorama import Fore, Back, Style
import numpy as np
import pandas as pd
pd.set_option('display.max_columns', None)
pd.set_option('display.max_rows', None)

import copy
import warnings
warnings.filterwarnings("ignore")

from pymongo import MongoClient
client=MongoClient(Config.DB_Hostname,Config.DB_Port)

class Directional_Signal_Strategizer:
    
    def __init__(self, parameters):
        
        '''              
        strategy_params = {"strategy_type": 1, "strategy_version": 1, "previous_close": 100, "position_parameter": 0.1,
        "extreme_move_parameter": 0.5, "abstinence_parameter": 0.15, "straight_abstinence_parameter": 0.11,
        "straight_reversal_parameter": 1/3, "directional_move_parameter": 0.4, "initial_move_parameter": 0.6,
        "take_profit_parameter": 0.15, "hedge_manage_parameter": 0.1, "stop_loss_parameter": 0.15, 
        "initial_time_parameter": 10, "current_week": 1}
        
        parameters = {"underlying": "NIFTY", "date": "2023-08-14", "strategy_variant": 1, "allocated_capital": 100, "num_legs": 2,
              "strategy_params": strategy_params, "instrument_list": instrument_list, "strike_multiples": 10, 
              "days_to_expiry" : {"2023-11-02":4,}} 
        
        '''
        
        self.parameters = copy.deepcopy(parameters)
        self.parameters['instrument_list'] = self._process_instrument_list(self.parameters['instrument_list'])
        self.allocated_capital = 0.975 * self.parameters["allocated_capital"]
        self.signal_generator = Directional_Signal_Generator.Directional_Signal_Generator(
                                self.parameters["strategy_params"]["strategy_type"], 
                                self.parameters["strategy_params"]["strategy_version"], 
                                self.parameters["strategy_params"]["previous_close"], 
                                self.parameters["strategy_params"]["position_parameter"], 
                                self.parameters["strategy_params"]["extreme_move_parameter"], 
                                self.parameters["strategy_params"]["abstinence_parameter"], 
                                self.parameters["strategy_params"]["straight_abstinence_parameter"], 
                                self.parameters["strategy_params"]["straight_reversal_parameter"], 
                                self.parameters["strategy_params"]["directional_move_parameter"], 
                                self.parameters["strategy_params"]["initial_move_parameter"], 
                                self.parameters["strategy_params"]["initial_time_parameter"])
        
        self.profit_leg = None
        self.hedge_leg = None
        self.direction = None
        self.portfolio = None
        self.take_profit = None
        self.book_hedge = None
        self.stop_loss = None
        self.portfolio_state = None #"OPEN", "HEDGE", "LOSS", "PROFIT", None
        self.complete_fill = None
        self.active_flag = True
        self.portfolio_id = 0
        self.drawdown_count = 0
        self.candle_flag = False
        self.pnl = 0
        
        #no backup required
        self.inputs = {'price_packet': None, 'price_guides': None, 'minute_flag': None, 'orders': None, 'update_greeks': None}
        self.itm_tolerance = {"NIFTY" : 12.5, "BANKNIFTY" : 25}
        
        #duplicate class variables for backup
        self.parameters_copy = None
        self.direction_copy = None
        self.portfolio_id_copy = None
        self.signal_generator_copy = None
        
        self.pnl_copy = None
        self.allocated_capital_copy = None
        self.profit_leg_copy = None
        self.hedge_leg_copy = None
        self.portfolio_copy = None
        self.complete_fill_copy = None
        self.active_flag_copy = None
        self.take_profit_copy = None
        self.book_hedge_copy = None
        self.stop_loss_copy = None
        self.portfolio_state_copy = None
        self.drawdown_count_copy = None
        self.candle_flag_copy = None
        self._backup()
        
        print(f"\nDirectional Signal Strategizer created for {self.parameters['underlying']} with parameters:")
        for k,v in self.parameters["strategy_params"].items():
            print(f"{k} : {v}")
        print(f"Allocated Capital: {self.allocated_capital}\n")

    def _backup(self):
        
        self.allocated_capital_copy = copy.deepcopy(self.allocated_capital)
        self.profit_leg_copy = copy.deepcopy(self.profit_leg)
        self.hedge_leg_copy = copy.deepcopy(self.hedge_leg)
        self.direction_copy = copy.deepcopy(self.direction)
        self.portfolio_copy = copy.deepcopy(self.portfolio)
        self.complete_fill_copy = copy.deepcopy(self.complete_fill)
        self.active_flag_copy = copy.deepcopy(self.active_flag)
        self.portfolio_id_copy = copy.deepcopy(self.portfolio_id)
        self.signal_generator_copy = copy.deepcopy(self.signal_generator)
        self.take_profit_copy = copy.deepcopy(self.take_profit)
        self.book_hedge_copy = copy.deepcopy(self.book_hedge)
        self.stop_loss_copy = copy.deepcopy(self.stop_loss)
        self.portfolio_state_copy = copy.deepcopy(self.portfolio_state)
        self.drawdown_count_copy = copy.deepcopy(self.drawdown_count)
        self.candle_flag_copy = copy.deepcopy(self.candle_flag)
        self.pnl_copy = copy.deepcopy(self.pnl)
        
    def Reset(self):
        
        self.allocated_capital = copy.deepcopy(self.allocated_capital_copy)
        self.profit_leg = copy.deepcopy(self.profit_leg_copy)
        self.hedge_leg = copy.deepcopy(self.hedge_leg_copy)
        self.direction = copy.deepcopy(self.direction_copy)
        self.portfolio = copy.deepcopy(self.portfolio_copy)
        self.complete_fill = copy.deepcopy(self.complete_fill_copy)
        self.active_flag = copy.deepcopy(self.active_flag_copy)
        self.portfolio_id = copy.deepcopy(self.portfolio_id_copy)
        self.signal_generator = copy.deepcopy(self.signal_generator_copy)
        self.take_profit = copy.deepcopy(self.take_profit_copy)
        self.book_hedge = copy.deepcopy(self.book_hedge_copy)
        self.stop_loss = copy.deepcopy(self.stop_loss_copy)
        self.portfolio_state = copy.deepcopy(self.portfolio_state_copy)
        self.drawdown_count = copy.deepcopy(self.drawdown_count_copy)
        self.candle_flag = copy.deepcopy(self.candle_flag_copy)
        self.pnl = copy.deepcopy(self.pnl_copy)
        
    def _process_instrument_list(self, instrument_list):
        
        condition = ((instrument_list.exchange == "NFO") & (instrument_list.underlying == self.parameters['underlying']) & 
        (instrument_list.instrument_type.isin(['CE', 'PE'])) & (instrument_list.strike % self.parameters["strike_multiples"] ==0))
        instrument_list = instrument_list[condition]
        
        instrument_list["expiry"] = [datetime.datetime.strptime(d,"%Y-%m-%d").date() for d in instrument_list.expiry]
        instrument_list["days_to_expiry"]= (pd.to_datetime(instrument_list.expiry)-pd.to_datetime(self.parameters["date"])).dt.days

        if self.parameters["strategy_params"]['current_week'] == 1:
            opt_expiry = sorted(instrument_list.days_to_expiry.unique())[:1]
        else:
            opt_expiry = sorted(instrument_list.days_to_expiry.unique())[1:2]
                
        return instrument_list[instrument_list.days_to_expiry.isin(opt_expiry)].drop(columns = ['days_to_expiry'])

    
    def _get_tradeable_instruments(self, underlying_price):
        
        instrument_list = copy.deepcopy(self.parameters['instrument_list'])
        days_to_expiry = self.parameters["days_to_expiry"][datetime.datetime.strftime(instrument_list.expiry.unique()[0], "%Y-%m-%d")]
        days_to_expiry -= (datetime.datetime.now().timestamp() - datetime.datetime.strptime(self.parameters["date"] + " 09:15:00", "%Y-%m-%d %H:%M:%S").timestamp()) / 22500
        
        instrument_list["strike_diff"] = instrument_list.strike - underlying_price * np.exp(Config.interest_rate * days_to_expiry / 252)
        instrument_list.strike_diff = np.where(abs(instrument_list.strike_diff) <= self.itm_tolerance[self.parameters['underlying']], 0, instrument_list.strike_diff)
        
        condition = ((instrument_list.instrument_type == 'CE')&(instrument_list.strike_diff >=0)) | ((instrument_list.instrument_type == 'PE')&(instrument_list.strike_diff <=0))
        instrument_list.strike_diff = abs(instrument_list.strike_diff)

        calls = instrument_list[condition&(instrument_list.instrument_type == "CE")].sort_values('strike_diff').head(self.parameters["num_legs"])
        puts = instrument_list[condition&(instrument_list.instrument_type == "PE")].sort_values('strike_diff').head(self.parameters["num_legs"])
        
        return calls.drop(columns = ['strike_diff']), puts.drop(columns = ['strike_diff'])

    def _get_instrument_weight_and_price(self, instr_list):
        
        weights = [0]*(len(instr_list))
        prices = [0]*(len(instr_list))
        
        for i, sym in enumerate(instr_list.tradingsymbol.tolist()):
            
            temp = np.array([[dict_["price"], dict_["quantity"]] for dict_ in self.inputs['price_packet'][sym]["depth"]["sell"]]).T
            price_array = temp[0]
            qty_array = temp[0]
            price_array[~np.isfinite(price_array)] = 0
            qty_array[~np.isfinite(qty_array)] = 0

            weights[i] = np.dot(price_array, qty_array)

            price_array = price_array[price_array!=0]
            if len(price_array) == 0:
                prices[i] = 0
            else:
                prices[i] = price_array[0]
                        
        weights = np.array(weights)/sum(weights)
        weights[~np.isfinite(weights)] = 0
        return weights, np.array(prices)

    def _get_qty(self):
                
        weights, prices = self._get_instrument_weight_and_price(self.profit_leg)
        self.profit_leg["weight"] = weights
        self.profit_leg["price"] = prices
        self.profit_leg.price = np.where(self.profit_leg.price == 0, self.profit_leg.tradingsymbol.map(self.inputs['price_guides']), self.profit_leg.price)

        weights, prices = self._get_instrument_weight_and_price(self.hedge_leg)
        self.hedge_leg["weight"] = weights
        self.hedge_leg["price"] = prices
        self.hedge_leg.price = np.where(self.hedge_leg.price == 0, self.hedge_leg.tradingsymbol.map(self.inputs['price_guides']), self.hedge_leg.price)
        
        self.profit_leg = self.profit_leg[self.profit_leg.weight != 0]
        self.hedge_leg = self.hedge_leg[self.hedge_leg.weight != 0]
        
        if len(self.profit_leg) * len(self.hedge_leg) == 0:
            raise RuntimeError("No Market Depth", self.parameters["strategy_variant"])
            
        else:
            self.profit_leg["filled_quantity"]= 0
            self.hedge_leg["filled_quantity"]= 0
            
        profit_allocated_capital = self.parameters["strategy_params"]["position_ratio"] * self.allocated_capital
        hedge_allocated_capital = (1 - self.parameters["strategy_params"]["position_ratio"]) * self.allocated_capital
            
        while True:
            
            self.profit_leg["quantity"] = self.profit_leg.apply(lambda x: x.lot_size * np.trunc(x.weight * profit_allocated_capital / (x.price * x.lot_size)), axis = 1)            
            self.hedge_leg["quantity"] = self.hedge_leg.apply(lambda x: x.lot_size * np.trunc(x.weight * hedge_allocated_capital / (x.price * x.lot_size)), axis = 1)
            
            if len(self.profit_leg[self.profit_leg.quantity==0])!=0 or len(self.hedge_leg[self.hedge_leg.quantity==0])!=0:
                self.profit_leg = self.profit_leg[self.profit_leg.quantity!=0]        
                self.hedge_leg = self.hedge_leg[self.hedge_leg.quantity!=0]
                
                if len(self.profit_leg) * len(self.hedge_leg) == 0:
                    raise RuntimeError("Insufficient capital allocation", self.parameters["strategy_variant"])
                    
                self.profit_leg.weight/=self.profit_leg.weight.sum()
                self.profit_leg.replace([np.nan, np.inf, -np.inf], 0, inplace = True)

                self.hedge_leg.weight/=self.hedge_leg.weight.sum()
                self.hedge_leg.replace([np.nan, np.inf, -np.inf], 0, inplace = True)

            else:
                break
    
    def _get_portfolio_id(self):
                
        if self.portfolio_id == 99:
            raise RuntimeError("Max portfolios reached", self.parameters["strategy_variant"])
            
        elif self.portfolio is not None:
            raise RuntimeError("Previous portfolio open", self.parameters["strategy_variant"])
            
        else:
            self.portfolio_id += 1
            
        return self.portfolio_id
            
    def _open_new_position(self, direction, signal):
    
        if direction == 1:
            self.profit_leg, self.hedge_leg = self._get_tradeable_instruments(self.inputs['price_packet'][self.parameters['underlying']])
        elif direction == -1:
            self.hedge_leg, self.profit_leg = self._get_tradeable_instruments(self.inputs['price_packet'][self.parameters['underlying']])
        else:
            raise RuntimeError("Unknown Trade Direction", self.parameters["strategy_variant"])

        if len(self.profit_leg) * len(self.hedge_leg) == 0:
            raise RuntimeError("Out of OTMs", self.parameters["strategy_variant"])

        #generating profit and hedge legs
        self._get_qty()
        self.direction = direction
        
        #combining profit and hedge legs
        combined_position = pd.concat([self.profit_leg, self.hedge_leg])
        combined_position["initial_price"] = 0
        combined_position["initial_quantity"] = 0
        combined_position["initial_underlying_price"] = 0
        combined_position["initial_IV"] = 0
        combined_position["initial_delta"] = 0
        combined_position["initial_gamma"] = 0
        combined_position["initial_vega"] = 0        
        combined_position["strategy_variant"] = self.parameters["strategy_variant"]
        combined_position["portfolio_id"] = self.portfolio_id
        
        #portfolio creation
        try:
            self.portfolio = Portfolio.Portfolio(self._get_portfolio_id(), self.parameters["strategy_variant"], 
                                                 combined_position.to_dict('records'), Config.interest_rate)
        except RuntimeError as e:
            raise RuntimeError("Could_not add_new instruments", self.portfolio_id)
        
        self.complete_fill = False 
        self.portfolio_state = "OPEN"
        self.take_profit = self.inputs['price_packet'][self.parameters['underlying']]*(1 + self.direction*self.parameters["strategy_params"]["take_profit_parameter"]*0.01)
        self.book_hedge = self.inputs['price_packet'][self.parameters['underlying']]*(1 - self.direction*self.parameters["strategy_params"]["hedge_manage_parameter"]*0.01)
        self.stop_loss = self.inputs['price_packet'][self.parameters['underlying']]*(1 - self.direction*self.parameters["strategy_params"]["stop_loss_parameter"]*0.01)
        
        combined_position.drop(columns =['initial_price', 'initial_quantity', 'initial_underlying_price', 'initial_IV', 'initial_delta', 'initial_gamma', 'initial_vega'], inplace = True)
        combined_position.expiry = pd.to_datetime(combined_position.expiry).apply(lambda x : datetime.datetime.strftime(x, "%Y-%m-%d"))
        combined_position["timestamp"] = datetime.datetime.strftime(datetime.datetime.now(), "%Y-%m-%d %H:%M:%S")
        
        if self.inputs["momentum_indicator"][self.parameters['underlying']] == 1:
            combined_position["priority"] = np.where(combined_position.tradingsymbol.str.endswith('CE'), 0, 1)
            
        elif self.inputs["momentum_indicator"][self.parameters['underlying']] == -1:
            combined_position["priority"] = np.where(combined_position.tradingsymbol.str.endswith('CE'), 1, 0)

        else:
            combined_position["priority"] = 0

        response = {"orders_to_execute": combined_position, "portfolio_log": None}
        return response
    
    def _order_update(self):
        
        if self.portfolio is None:
            raise RuntimeError("No Existing Portfolio", self.parameters["strategy_variant"])
        else:
            try:
                portfolio_log = self.portfolio.Update_Order(self.inputs['orders'], self.inputs['price_guides'], False)
            except RuntimeError as e:
                raise RuntimeError(e.args[0], e.args[1])
                
        order_df = pd.DataFrame(self.inputs['orders'])
        symbol_quantity_map = order_df[['tradingsymbol', 'quantity']].groupby('tradingsymbol').sum().quantity.to_dict()
            
        self.profit_leg["filled_quantity_temp"]= self.profit_leg.tradingsymbol.map(symbol_quantity_map)
        self.hedge_leg["filled_quantity_temp"]= self.hedge_leg.tradingsymbol.map(symbol_quantity_map)
        self.profit_leg.fillna(0, inplace=True)
        self.hedge_leg.fillna(0, inplace=True)
        self.profit_leg.filled_quantity += self.profit_leg.filled_quantity_temp
        self.hedge_leg.filled_quantity += self.hedge_leg.filled_quantity_temp
        self.profit_leg.drop(columns=['filled_quantity_temp'], inplace=True)
        self.hedge_leg.drop(columns=['filled_quantity_temp'], inplace=True)
        
        if sum(abs(self.profit_leg.quantity - self.profit_leg.filled_quantity)) == 0 and sum(abs(self.hedge_leg.quantity - self.hedge_leg.filled_quantity)) == 0:
            self.complete_fill = True
            if self.portfolio_state == "PROFIT" or self.portfolio_state == "LOSS":
                response = self._manage_intermediate_position()
                return response
            
        response = {"orders_to_execute": None, "portfolio_log": None}
        return response
            
    def _calculate_pnl(self):
        
        if self.portfolio is None:
            raise RuntimeError("No Existing Portfolio", self.parameters["strategy_variant"])

        try:
            portfolio_log = self.portfolio.Calculate_Pnl(self.inputs['price_guides'], self.inputs['update_greeks'])
        except Exception as e:
            print(f"{e}, Could not calculate PnL for {self.parameters['underlying']} portfolio with id \
                  {self.portfolio_id} and strategy_variant ", self.parameters["strategy_variant"])
            portfolio_log = None
            
        response = {"orders_to_execute": None, "portfolio_log": portfolio_log}
        return response
    
    def Pretty_Print(self):
                    
        print(Fore.BLACK, Back.LIGHTYELLOW_EX, self.parameters["strategy_variant"], self.parameters["underlying"], f"-> Active Status: {self.active_flag}")
        print("\nProfit Leg: ")
        print(self.profit_leg)
        print("\nHedge Leg: ")
        print(self.hedge_leg)
        print("\nSignal Generator Status")
        print(f"Direction : {self.signal_generator.direction}, Signal : {self.signal_generator.signal}, Position_Parameter : {self.signal_generator.position_parameter}, Directional_Parameter : {self.signal_generator.directional_move_parameter}")
        print(f"Critical_Point : {self.signal_generator.critical_point}, Position_Flag : {self.signal_generator.position_flag}, Straight_Move_Flag : {self.signal_generator.straight_move_flag}, Extreme_Move_Flag : {self.signal_generator.extreme_move_flag}")
        print(Style.RESET_ALL)
        
    def Update(self, inputs):
        
        '''
        _calculate_pnl(price_dict, update_greeks = True):
        _open_new_position(price_packet, price_guides, minute_flag = False)
        orders#minute_flag#settle_for_less
        
        inputs = {'price_packet':, 'price_guides':, 'minute_flag':, 'orders':, 'update_greeks':, 'momentum_indicator':}
        orders = [{"order_id":,"tradingsymbol":"","quantity":,"price":,"timestamp":},{},{}] / None
        price_guides = {"200CE":2,"100CE":1.5,"200PE":1.25,"100PE":0.75, "NIFTY":100.75,"BANKNIFTY":200.75} / None
        minute_flag/ update_greeks -> True / False/ None
        '''
        self._backup()
        self.inputs = copy.deepcopy(inputs)
        
        response = {"orders_to_execute": None, "portfolio_log": None}
        if not self.active_flag:
            return response
                
        if self.inputs['orders'] is not None:
            
            if self.inputs['price_guides'] is None:
                raise RuntimeError("price_guides missing_in order_update", self.portfolio_id)
            else:
                response = self._order_update()
                
        elif self.inputs['minute_flag'] is not None:
            
            if self.inputs['price_guides'] is None:
                raise RuntimeError("price_guides missing_in price_update", self.portfolio_id)
            
            elif self.inputs['update_greeks'] is None:
                raise RuntimeError("update_greeks missing_in price_update", self.portfolio_id)
            
            elif self.inputs['price_packet'] is None:
                raise RuntimeError("price_packet missing_in price_update", self.portfolio_id)
                
            elif self.inputs['minute_flag']:
                                
                direction, signal = self.signal_generator.Update(self.inputs['price_packet']["batch_id"], self.inputs['price_packet'][self.parameters['underlying']], critical = self.candle_flag)
                if self.candle_flag:
                    self.candle_flag = False
                
                if direction * signal != 0 and self.portfolio is None:
                    response = self._open_new_position(direction, signal)
                                        
                elif self.portfolio is not None:
                    response = self._manage_intermediate_position()
                
            elif self.portfolio is not None:
                response = self._manage_intermediate_position()
            
        else:
            raise RuntimeError("update missing order_or_minute", self.portfolio_id)

        return response

    def _manage_intermediate_position(self):
        
        combined_position = None
        portfolio_log = None
        if self.complete_fill:
            
            if self.portfolio_state == "OPEN":
                                                            
                if self.direction*self.inputs['price_packet'][self.parameters['underlying']] >= self.direction*self.take_profit:

                    self.portfolio_state = "PROFIT"
                    combined_position = self._transition()
                    self.complete_fill= False
                    self.candle_flag = True
                    
                elif self.direction*self.inputs['price_packet'][self.parameters['underlying']] <= self.direction*self.stop_loss:

                    self.portfolio_state = "LOSS"
                    combined_position = self._transition()
                    self.complete_fill= False
                    self.candle_flag = True
                    
                elif self.direction*self.inputs['price_packet'][self.parameters['underlying']] <= self.direction*self.book_hedge:
                    
                    self.portfolio_state = "HEDGE"
                    combined_position = self._transition()
                    self.complete_fill= False
            
            elif self.portfolio_state == "HEDGE":
                
                if self.direction*self.inputs['price_packet'][self.parameters['underlying']] >= self.direction*self.take_profit:
                
                    self.portfolio_state = "PROFIT"
                    combined_position = self._transition()
                    self.complete_fill= False
                    self.candle_flag = True
                
                elif self.direction*self.inputs['price_packet'][self.parameters['underlying']] <= self.direction*self.stop_loss:
                    
                    self.portfolio_state = "LOSS"
                    combined_position = self._transition()
                    self.complete_fill= False
                    self.candle_flag = True

            else:
                portfolio_log = self._close_position()
        
        if portfolio_log is None:
            portfolio_log = self._calculate_pnl()["portfolio_log"]
            
        response = {"orders_to_execute": combined_position, "portfolio_log": portfolio_log}
        return response
    
    def _transition(self):
                
        if self.portfolio_state == "PROFIT" or self.portfolio_state == "LOSS":
            
            if self.portfolio_state_copy == "OPEN":
                
                self.profit_leg.quantity *= -1
                self.hedge_leg.quantity *= -1
                self.hedge_leg.filled_quantity = 0
                self.profit_leg.filled_quantity = 0
                combined_position = pd.concat([self.profit_leg, self.hedge_leg])
                
                if self.inputs["momentum_indicator"][self.parameters['underlying']] * self.direction >= 0:
                    combined_position["priority"] = np.where(combined_position.tradingsymbol.isin(self.profit_leg.tradingsymbol), 1, 0)
                
                else:
                    combined_position["priority"] = np.where(combined_position.tradingsymbol.isin(self.profit_leg.tradingsymbol), 0, 1)
                    
            else:
                self.profit_leg.quantity *= -1
                self.profit_leg.filled_quantity = 0
                combined_position = copy.deepcopy(self.profit_leg)
                combined_position["priority"]=0
                                            
        elif self.portfolio_state == "HEDGE":
            
            if self.parameters["strategy_params"]["strategy_type"] == 1:
                
                self.hedge_leg.filled_quantity = 0
                self.hedge_leg.quantity *= -1
                combined_position = copy.deepcopy(self.hedge_leg)
                combined_position["priority"]=0
            
            else:

                self.hedge_leg.filled_quantity = 0
                self.hedge_leg.quantity *= -1
                
                combined_position = self._manage_hedge_strategy_type_2()
                if combined_position is None:
                    combined_position = copy.deepcopy(self.hedge_leg)
                else:
                    combined_position = pd.concat([self.hedge_leg, combined_position])
                    
                combined_position["priority"] = np.where(combined_position.tradingsymbol.isin(self.profit_leg.tradingsymbol), 1, 0)
        
        combined_position["strategy_variant"] = self.parameters["strategy_variant"]
        combined_position["underlying"] = self.parameters["underlying"]
        return combined_position[combined_position.quantity!=combined_position.filled_quantity]
                
    def _manage_hedge_strategy_type_2(self):

        if self.direction == 1:
            profit_leg, _ = self._get_tradeable_instruments(self.inputs['price_packet'][self.parameters['underlying']])
        elif self.direction == -1:
            _, profit_leg = self._get_tradeable_instruments(self.inputs['price_packet'][self.parameters['underlying']])
        
        profit_leg["weight"] = 0
        profit_leg["price"] = 0
        profit_leg["quantity"] = 0
        profit_leg["filled_quantity"] = 0
        profit_leg["strategy_variant"] = self.parameters["strategy_variant"]
        
        extra_sym = []
        for sym in profit_leg.tradingsymbol:
            if sym not in self.profit_leg.tradingsymbol.tolist():
                extra_sym.append(sym)

        #adding instruments to portfolio
        if len(extra_sym) != 0:

            profit_leg_temp = profit_leg[profit_leg.tradingsymbol.isin(extra_sym)]
            profit_leg_temp["initial_price"] = 0
            profit_leg_temp["initial_quantity"] = 0
            profit_leg_temp["initial_underlying_price"] = 0
            profit_leg_temp["initial_IV"] = 0
            profit_leg_temp["initial_delta"] = 0
            profit_leg_temp["initial_gamma"] = 0
            profit_leg_temp["initial_vega"] = 0        
            profit_leg_temp["portfolio_id"] = self.portfolio_id

            try:
                self.portfolio.Add_Instruments(profit_leg_temp.to_dict('records'))
            except RuntimeError as e:
                raise RuntimeError("Could_not add_new instruments", self.portfolio_id)

        profit_leg.weight, profit_leg.price = self._get_instrument_weight_and_price(profit_leg)
        profit_leg.price = np.where(profit_leg.price == 0, profit_leg.tradingsymbol.map(self.inputs['price_guides']), profit_leg.price)
        profit_leg = profit_leg[profit_leg.weight != 0]

        if len(profit_leg) != 0:

            while True:

                profit_leg.quantity = profit_leg.apply(lambda x: x.lot_size * np.trunc(
                (1 - self.parameters["strategy_params"]["position_ratio"]) * self.allocated_capital * x.weight/(x.price * x.lot_size)), axis = 1)
                if len(profit_leg[profit_leg.quantity==0]) != 0:
                    profit_leg = profit_leg[profit_leg.quantity != 0]
                    profit_leg.weight /= profit_leg.weight.sum()
                    profit_leg.replace([np.nan, np.inf, -np.inf], 0, inplace = True)

                else:
                    break
            
            self.profit_leg = pd.concat([self.profit_leg, profit_leg[~profit_leg.tradingsymbol.isin(self.profit_leg.tradingsymbol)]])
            self.profit_leg["additional_quantity"]= self.profit_leg.tradingsymbol.map(profit_leg.set_index('tradingsymbol').quantity.to_dict())
            self.profit_leg.fillna(0, inplace=True)
            self.profit_leg.quantity = np.where(self.profit_leg.tradingsymbol.isin(extra_sym), self.profit_leg.quantity, self.profit_leg.quantity + self.profit_leg.additional_quantity)
            self.profit_leg.drop(columns = ['additional_quantity'], inplace=True)

        if len(profit_leg) !=0:
            return profit_leg
        else:
            return None

    def _close_position(self):
        
        portfolio_log = self._calculate_pnl()["portfolio_log"]
        self.pnl += portfolio_log['portfolio_pnl']
        self.allocated_capital = min(self.allocated_capital, self.allocated_capital + self.pnl)

        if self.portfolio_state == "LOSS":
            
            self.drawdown_count += 1
            if self.drawdown_count == 2:
                self.active_flag = False
                                                
        self.profit_leg = None
        self.hedge_leg = None        
        self.portfolio = None
        self.complete_fill = None                
        self.take_profit = None
        self.book_hedge = None
        self.stop_loss = None
        self.portfolio_state = None
        
        portfolio_log['portfolio_pnl'] = 0
        return portfolio_log