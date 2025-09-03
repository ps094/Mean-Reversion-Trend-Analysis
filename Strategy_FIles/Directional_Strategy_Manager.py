import Directional_Signal_Generator
import pandas as pd
import datetime
import BS
from pymongo import MongoClient
import Config
client = MongoClient(Config.DB_Hostname, Config.DB_Port)

class Directional_Strategy_Manager:
    
    def __init__(self, Date_Today, Strategy_Params):
        
        # strategy params -> strategy_type(1 safe hedging vs 2 risky), strategy_version, previous_close, position_parameter,
        # extreme_move_parameter, abstinence_parameter, straight_abstinence_parameter, straight_reversal_parameter,
        # directional_move_parameter, initial_move_parameter, take_profit_parameter, hedge_manage_parameter,
        # stop_loss_parameter, position_ratio, initial_time_parameter 
        
        self.date_today=Date_Today #2020-03-06 format
        self.strategy_params=Strategy_Params

        self.Signal_Generator=Directional_Signal_Generator.Directional_Signal_Generator(self.strategy_params["strategy_type"], self.strategy_params["strategy_version"], self.strategy_params["previous_close"], self.strategy_params["position_parameter"], self.strategy_params["extreme_move_parameter"], self.strategy_params["abstinence_parameter"], self.strategy_params["straight_abstinence_parameter"], self.strategy_params["straight_reversal_parameter"], self.strategy_params["directional_move_parameter"], self.strategy_params["initial_move_parameter"], self.strategy_params["initial_time_parameter"])    
    
    def test(self, prices, current_week=True, strike_offset=0.1, vol=0.2, days_to_expiry=5):
        
        try:
            capital=100000
            pnl=[0]*375
            drawdown_count=0
            trade_count=0
            position_flag=0
            direction=0
            signal=0
            call_value=0
            put_value=0
            call_price=0
            put_price=0
            call_pos=0
            put_pos=0
            call_strike=0
            put_strike=0
            previous_close=0
            constant=0
            trade_batch=0
            first_drawdown_pnl=0
            second_draw_down_pnl=0

            time_to_expiry=days_to_expiry
            trade_time=time_to_expiry
            
            for open_price,high_price,low_price,close_price,batch_id in zip(prices.open,prices.high,prices.low,prices.close,prices.batch_id):

                if batch_id>344:
                    trade_time=time_to_expiry-batch_id/375
                    call_value=BS.bs_call(close_price,call_strike,trade_time/252,0.07,vol)
                    put_value=BS.bs_put(close_price,put_strike,trade_time/252,0.07,vol)
                    capital+=call_pos*call_value + put_pos*put_value
                    pnl[batch_id-1]+=call_pos*(call_value-call_price) + put_pos*(put_value-put_price)
                    position_flag=0
                    call_pos=0
                    put_pos=0
                    break

                if direction*signal!=0 and position_flag==0:

                    trade_time=time_to_expiry-batch_id/375
                    trade_batch=batch_id
                    previous_close=self.Signal_Generator.close
                    call_strike=previous_close*(1+strike_offset/100)
                    put_strike=previous_close*(1-strike_offset/100)                
                    call_price=BS.bs_call(previous_close,call_strike,trade_time/252,0.07,vol)
                    put_price=BS.bs_put(previous_close,put_strike,trade_time/252,0.07,vol)

                    if direction > 0:
                        constant=min(capital,100000)/(self.strategy_params["position_ratio"] * call_price+(1-self.strategy_params["position_ratio"])*put_price) 
                        call_pos=constant*self.strategy_params["position_ratio"]
                        put_pos=constant*(1-self.strategy_params["position_ratio"])
                        exit_price=previous_close*(1 + self.strategy_params["take_profit_parameter"]*0.01)
                        hedge_price=previous_close*(1 - self.strategy_params["hedge_manage_parameter"]*0.01)
                        stop_loss=previous_close*(1 - self.strategy_params["stop_loss_parameter"]*0.01)
                        position_flag=1

                    else:
                        constant=min(capital,100000)/((1- self.strategy_params["position_ratio"])*call_price+self.strategy_params["position_ratio"]*put_price)
                        call_pos=constant*(1-self.strategy_params["position_ratio"])
                        put_pos=constant*self.strategy_params["position_ratio"]
                        exit_price=previous_close*(1 - self.strategy_params["take_profit_parameter"]*0.01)
                        hedge_price=previous_close*(1 + self.strategy_params["hedge_manage_parameter"]*0.01)
                        stop_loss=previous_close*(1 + self.strategy_params["stop_loss_parameter"]*0.01)
                        position_flag=-1

                    trade_count+=1
                    capital-=call_pos*call_price+put_pos*put_price
                    direction,signal=self.Signal_Generator.Update(batch_id,close_price)
                    continue

                if position_flag>0: 

                    exit_time=trade_time - (batch_id-trade_batch)/375

                    if low_price<=hedge_price:

                        if put_pos!=0:

                            put_value=BS.bs_put(hedge_price,put_strike,exit_time/252,0.07,vol)

                            if self.strategy_params["strategy_type"]==2 and put_pos!=0:
                                call_value=BS.bs_call(hedge_price,call_strike,exit_time/252,0.07,vol)
                                call_price=(call_price*call_pos+put_pos*put_value)/(call_pos+(put_pos*put_value/call_value))
                                call_pos+=put_pos*put_value/call_value
                            else:
                                capital+=put_pos*put_value

                            pnl[batch_id-1]+=put_pos*(put_value-put_price)
                            put_pos=0

                        else:
                            put_value=0

                        if low_price<=stop_loss:

                            call_value=BS.bs_call(stop_loss,call_strike,exit_time/252,0.07,vol)                            
                            capital+=call_pos*call_value + put_pos*put_value
                            pnl[batch_id-1]+=call_pos*(call_value-call_price) + put_pos*(put_value-put_price)
                            position_flag=0
                            call_pos=0
                            put_pos=0
                            
                            if drawdown_count<=1:
                                if drawdown_count==0:
                                    first_drawdown_pnl=sum(pnl)
                                second_draw_down_pnl=sum(pnl)
                                
                            drawdown_count+=1
                            direction,signal=self.Signal_Generator.Update(batch_id,close_price,critical=True)
                            continue

                        if high_price<exit_price:
                            direction,signal=self.Signal_Generator.Update(batch_id,close_price)
                            continue

                    if high_price>=exit_price:

                        call_value=BS.bs_call(exit_price,call_strike,exit_time/252,0.07,vol)
                        put_value=BS.bs_put(exit_price,put_strike,exit_time/252,0.07,vol)
                        capital+=call_pos*call_value + put_pos*put_value
                        pnl[batch_id-1]+=call_pos*(call_value-call_price) + put_pos*(put_value-put_price)
                        position_flag=0
                        call_pos=0
                        put_pos=0
                        direction,signal=self.Signal_Generator.Update(batch_id,close_price,critical=True)
                        continue

                elif position_flag<0:

                    exit_time=trade_time - (batch_id-trade_batch)/375

                    if high_price>=hedge_price:

                        if call_pos!=0:

                            call_value=BS.bs_call(hedge_price,call_strike,exit_time/252,0.07,vol)

                            if self.strategy_params["strategy_type"]==2 and call_pos!=0:
                                put_value=BS.bs_put(hedge_price,put_strike,exit_time/252,0.07,vol)
                                put_price=(put_price*put_pos+call_pos*call_value)/(put_pos+(call_pos*call_value/put_value))
                                put_pos+=call_pos*call_value/put_value
                            else:
                                capital+=call_pos*call_value
                            pnl[batch_id-1]+=call_pos*(call_value-call_price)
                            call_pos=0

                        else:
                            call_value=0

                        if high_price>=stop_loss:

                            put_value=BS.bs_put(stop_loss,put_strike,exit_time/252,0.07,vol)                            
                            capital+=put_pos*put_value + call_pos*call_value
                            pnl[batch_id-1]+=put_pos*(put_value-put_price) + call_pos*(call_value-call_price)
                            position_flag=0
                            put_pos=0
                            call_pos=0
                            
                            if drawdown_count<=1:
                                if drawdown_count==0:
                                    first_drawdown_pnl=sum(pnl)
                                second_draw_down_pnl=sum(pnl)

                            drawdown_count+=1                        
                            direction,signal=self.Signal_Generator.Update(batch_id,close_price,critical=True)
                            continue

                        if low_price>exit_price:
                            direction,signal=self.Signal_Generator.Update(batch_id,close_price)
                            continue

                    if low_price<=exit_price:

                        put_value=BS.bs_put(exit_price,put_strike,exit_time/252,0.07,vol)
                        call_value=BS.bs_call(exit_price,call_strike,exit_time/252,0.07,vol)
                        capital+=put_pos*put_value + call_pos*call_value
                        pnl[batch_id-1]+=put_pos*(put_value-put_price) + call_pos*(call_value-call_price)
                        position_flag=0
                        put_pos=0
                        call_pos=0
                        direction,signal=self.Signal_Generator.Update(batch_id,close_price,critical=True)
                        continue

                
                direction,signal=self.Signal_Generator.Update(batch_id,close_price)
                continue
            
            if drawdown_count==0:
                first_drawdown_pnl=sum(pnl)
                second_draw_down_pnl=first_drawdown_pnl
            elif drawdown_count==1:
                second_draw_down_pnl=sum(pnl)

            summary={"PnL":sum(pnl), "Drawdown_Count":drawdown_count, "Trade_Count":trade_count, "first_drawdown_pnl":first_drawdown_pnl, "second_drawdown_pnl":second_draw_down_pnl}
            return summary
        
        except Exception as e:
            print(e)
            summary={"PnL":0,"Drawdown_Count":0,"Trade_Count":0,"first_drawdown_pnl":0,"second_drawdown_pnl":0}
            return summary

def Test(strategy_params,Prices,date_today):
    """
    strategy_params={"strategy_type":strategy_type, "strategy_version":strategy_version, "previous_close":9100.40,
    "position_parameter":position_param, "extreme_move_parameter":1.1*move_param,"abstinence_parameter":1.5*position_param,
    "straight_abstinence_parameter":1.1* position_param, "straight_reversal_parameter":position_param/2,
    "directional_move_parameter":move_param,"initial_move_parameter":1.5*move_param, "initial_time_parameter":10,
    "take_profit_parameter":profit_param, "hedge_manage_parameter":0.5*profit_param,"stop_loss_parameter":profit_param*2,
    "position_ratio":position_ratio,"current_week":current_week,"sim_vol":0.0995,"days_to_expiry":3}
    """
    Strategy=Directional_Strategy_Manager(date_today,strategy_params)
    return Strategy.test(Prices,current_week = strategy_params["current_week"], strike_offset=0.1, vol = strategy_params["sim_vol"], days_to_expiry = strategy_params["days_to_expiry"])
            