import Config
import numpy as np
import pandas as pd
pd.set_option('display.float_format', lambda x: '%.5f' % x)
pd.set_option('display.max_columns', None)
import matplotlib.pyplot as plt
import warnings
warnings.filterwarnings("ignore")

import copy
import time
from pymongo import MongoClient
client=MongoClient(Config.DB_Hostname,Config.DB_Port)

def Simulate(params):
    #params {"simulation_parameters":{}, "parameters": {}, identifier: 1/2/3...}
    Filter = Live_Strategy_Filter(parameters = params["parameters"], simulation_parameters = params["simulation_parameters"])
    sim_results = Filter.Simulate(full_sim=True)
    sim_results["identifier"] = params["identifier"]
    return sim_results

class Live_Strategy_Filter:
    
    def __init__(self, parameters = None, simulation_parameters = None):
        
        '''
        parameters = {"start_date" : "2022-04-22",
                      "end_date" : "2023-10-12", 
                      "prediction_date" : '2023-10-09'}
                    
        simulation_parameters : {"rolling_periods" : -1, 
                                 "zero_rtd_flag" : False,
                                 "outlier_flag": True, 
                                 "train_sample_size" : 20, 
                                 "maximize_returns" : True, 
                                 "test_sample_size" : 5}
        '''
        self.parameters = copy.deepcopy(parameters)
        if simulation_parameters is None:
            self._get_profit_maximizing_hyperparams()
        else:
            self.simulation_parameters = copy.deepcopy(simulation_parameters)
        
        self.profit_maximizing_params = None
        self.Live_Strategies = None

        #process live strategies        
        self.Live_Strategies=pd.DataFrame(client.Strategy.Live_Strategies.find()).drop(columns = ['_id'])
        self.Live_Strategies = self.Live_Strategies[(self.Live_Strategies.date >= self.parameters["start_date"]) & (self.Live_Strategies.date <= self.parameters["end_date"])]
        self.Live_Strategies.sort_values(['date','strategy_variant'], inplace = True)             
        self._process_live_strategies()
                
        #week_variant stratifications
        self.current_week_strategy_variants = []
        self.next_week_strategy_variants = []
        self._get_week_stratifications()
        print("Live_Strategy_Filter Initiated", flush=True)

    def _get_profit_maximizing_hyperparams(self):

        params = pd.DataFrame(client.Strategy.Profit_Maximizing_Hyperparams.find()).drop(columns =['_id'])
        params = params.sort_values('date', ascending=False).head(1)[['rolling_periods', 'zero_rtd_flag', 'outlier_flag', 'train_sample_size', 'test_sample_size', 'maximize_returns', 'top_n']].to_dict('records')[0]
        self.simulation_parameters = copy.deepcopy(params)
        
    def _get_profit_maximizing_params(self):#tbd
        
        params = pd.DataFrame(client.Strategy.Profit_Maximizing_Params.find()).drop(columns =['_id'])
        latest_date = sorted(params.date.unique())[-1]
        params = params[params.date == latest_date].sort_values('priority')[['current_week_limit', 'next_week_limit', 'rtd_threshold_lower', 'rtd_threshold_upper']].to_dict('records')
        self.profit_maximizing_params = copy.deepcopy(params)
        
    def _get_week_stratifications(self):
        
        for strategy_variant in range(1, self.Live_Strategies.strategy_variant.max() + 1):
            
            if (strategy_variant - 1) % 24 <= 11:
                self.current_week_strategy_variants.append(strategy_variant)
            else:
                self.next_week_strategy_variants.append(strategy_variant)
    
    def _process_live_strategies(self):
        
        Live_Strategies = []
        for strategy_variant in range(1, self.Live_Strategies.strategy_variant.max() + 1):
            
            temp = self.Live_Strategies[self.Live_Strategies.strategy_variant == strategy_variant]
            temp["rtd"] = 1 + temp.returns
            
            if self.simulation_parameters['rolling_periods'] == -1:
                temp.rtd = temp.rtd.expanding(min_periods=250).apply(lambda x: x.prod(), raw=True).shift()
                temp["num_predictions_till_date"] = temp.prediction.expanding(min_periods=250).sum().shift()
                
            else:
                temp.rtd = temp.rtd.rolling(self.simulation_parameters['rolling_periods']).apply(lambda x: x.prod(), raw=True).shift()
                temp["num_predictions_till_date"] = temp.prediction.rolling(self.simulation_parameters['rolling_periods']).sum().shift()
                
            Live_Strategies.append(temp)

        self.Live_Strategies = pd.concat(Live_Strategies).dropna().sort_values(['date','strategy_variant'])

    def _generate_prediction(self, date, current_week_limit, next_week_limit, rtd_threshold_upper, rtd_threshold_lower):

        #filter for the respective date
        daily_data = self.Live_Strategies[self.Live_Strategies.date == date]
#        return daily_data[daily_data.prediction==1]
        if self.simulation_parameters["zero_rtd_flag"]:
            daily_data = daily_data[daily_data.rtd != 1]
        
        #filter for the current week and next week
        current_week_daily_data = daily_data[daily_data.strategy_variant.isin(self.current_week_strategy_variants)]
        next_week_daily_data = daily_data[daily_data.strategy_variant.isin(self.next_week_strategy_variants)]

        #calculating rtd quantiles
        current_week_rtd_threshold_lower = current_week_daily_data.rtd.quantile(rtd_threshold_lower/100)
        current_week_rtd_threshold_upper = current_week_daily_data.rtd.quantile(rtd_threshold_upper/100)
        next_week_rtd_threshold_lower = next_week_daily_data.rtd.quantile(rtd_threshold_lower/100)
        next_week_rtd_threshold_upper = next_week_daily_data.rtd.quantile(rtd_threshold_upper/100)
        
        #filter for rtd thresholds
        current_week_daily_data = current_week_daily_data[(current_week_daily_data.rtd >= current_week_rtd_threshold_lower) & (current_week_daily_data.rtd <= current_week_rtd_threshold_upper)]
        next_week_daily_data = next_week_daily_data[(next_week_daily_data.rtd >= next_week_rtd_threshold_lower) & (next_week_daily_data.rtd <= next_week_rtd_threshold_upper)]

        #filter for predicted strategies
        current_week_daily_data = current_week_daily_data[current_week_daily_data.prediction == 1]
        next_week_daily_data = next_week_daily_data[next_week_daily_data.prediction == 1]

        #filter for top startegies in the bucket
        current_week_daily_data = current_week_daily_data.sort_values(['rtd', 'num_predictions_till_date'], ascending = [False, False]).head(current_week_limit)
        next_week_daily_data = next_week_daily_data.sort_values(['rtd', 'num_predictions_till_date'], ascending = [False, False]).head(next_week_limit)
        
        #collate
        daily_data  = pd.concat([current_week_daily_data, next_week_daily_data]).sort_values(['rtd', 'num_predictions_till_date'], ascending = [True, True])
        
        if self.simulation_parameters["outlier_flag"]:
            daily_data = daily_data.head(max(len(daily_data) - 1, 1))

        return daily_data
    
    def Simulate(self, full_sim = False):
        
        if full_sim:
            date_list = sorted(self.Live_Strategies.date.unique())
        else:
            split_date = sorted(self.Live_Strategies.date.unique())[-self.simulation_parameters["train_sample_size"]]
            date_list = sorted(self.Live_Strategies[self.Live_Strategies.date >= split_date].date.unique())
            
        sim = []
        dummy_entry = pd.DataFrame([{'date': 'DUMMY', 'underlying': 'DUMMY', 'strategy_variant': 0, 'pos_param': 0, 'mov_param': 0,
                                     'prof_param': 0, 'prediction': 0, 'returns': 0, 'rtd': 0, 'num_predictions_till_date': 0}])

        for i in range(5):#current week
            
            current_week_limit = i
            for j in range(5): #next week
                
                next_week_limit = j
                if not full_sim:
                    print(f"current_week_limit : {current_week_limit}, next_week_limit : {next_week_limit}")
                
                for rtd_threshold_upper in list(range(20, 101, 20)): #rtd_threshold_upper

                    for rtd_threshold_lower in range(0, rtd_threshold_upper -1, 20): #rtd_threshold_lower
                        
                        for date in date_list:
                            
                            temp = self._generate_prediction(date, current_week_limit, next_week_limit, rtd_threshold_upper, rtd_threshold_lower)
                            if len(temp) == 0:
                                
                                temp = copy.deepcopy(dummy_entry)
                                temp.date = [date]

                            temp["current_week_limit"] = current_week_limit
                            temp["next_week_limit"] = next_week_limit
                            temp["rtd_threshold_lower"] = rtd_threshold_lower
                            temp["rtd_threshold_upper"] = rtd_threshold_upper
                            sim.append(temp)
        
        return pd.concat(sim).sort_values('date')
    
    def _calc_portfolio_performance(self, temp_sim, split_dates):
        
        if split_dates is None:

            returns = temp_sim.sort_values('date').groupby('date').mean().returns
            drawdown = copy.deepcopy(returns)
            drawdown[drawdown > 0] = 0
            return {"average_returns" : np.mean(returns),
                    "absolute_return_volatility" : np.std(returns),
                    "average_MDD" : np.mean(drawdown)}
        
        else:
            split_dates = sorted(split_dates)
            average_returns_list = []
            average_MDD_list =[]

            for i in range(len(split_dates)):

                if i==0:
                    temp = temp_sim[temp_sim.date <= split_dates[i]]
                else:
                    temp = temp_sim[(temp_sim.date > split_dates[i-1]) & (temp_sim.date <= split_dates[i])]

                cumulative_returns = (1 + temp.groupby('date').mean().returns).cumprod()
                average_MDD = min(-1 + cumulative_returns/cumulative_returns.cummax().apply(lambda x: max(1, x)))
                average_returns = cumulative_returns.tolist()[-1]**(1/len(temp.date.unique())) - 1                
                average_returns_list.append(average_returns)
                average_MDD_list.append(average_MDD)

            return {"average_returns" : np.mean(average_returns_list),
                    "absolute_return_volatility" : np.std(average_returns_list),
                    "average_MDD" : np.mean(average_MDD_list)}

    def _filter_best_micro_params(self, simulation, maximize_returns = True, top_n = 1):

        sim = copy.deepcopy(simulation)
        current_week_limit_list = []
        next_week_limit_list = []
        rtd_threshold_lower_list = []
        rtd_threshold_upper_list = []

        average_returns = []
        absolute_return_volatility = []
        average_MDD = []

        performance = pd.DataFrame()
        date_list = sorted(sim.date.unique())
        split_dates = None

        for current_week_limit in sorted(sim.current_week_limit.unique().tolist()):
            for next_week_limit in sorted(sim.next_week_limit.unique().tolist()):
                for rtd_threshold_lower, rtd_threshold_upper in sorted(set((zip(sim.rtd_threshold_lower, sim.rtd_threshold_upper)))):

                    current_week_limit_list.append(current_week_limit)
                    next_week_limit_list.append(next_week_limit)
                    rtd_threshold_lower_list.append(rtd_threshold_lower)
                    rtd_threshold_upper_list.append(rtd_threshold_upper)

                    temp_sim = sim[(sim.current_week_limit == current_week_limit) & 
                                   (sim.next_week_limit == next_week_limit) & 
                                   (sim.rtd_threshold_lower == rtd_threshold_lower) &
                                   (sim.rtd_threshold_upper == rtd_threshold_upper)]

                    perf = self._calc_portfolio_performance(temp_sim, split_dates)
                    average_returns.append(perf["average_returns"])
                    absolute_return_volatility.append(perf["absolute_return_volatility"])
                    average_MDD.append(perf["average_MDD"])

        performance["current_week_limit"] = current_week_limit_list
        performance["next_week_limit"] = next_week_limit_list
        performance["rtd_threshold_lower"] = rtd_threshold_lower_list
        performance["rtd_threshold_upper"] = rtd_threshold_upper_list
        performance["average_returns"] = average_returns
        performance["absolute_return_volatility"] = absolute_return_volatility
        performance["average_MDD"] = average_MDD        
        performance["sharpe"] = np.where(performance.absolute_return_volatility == 0, 
                                         np.where(performance.average_returns == 0, 0, np.where(performance.average_returns>0, np.inf, -np.inf)), 
                                         performance.average_returns / performance.absolute_return_volatility)
        
        performance = performance[performance.average_MDD >= performance.average_MDD.quantile(0.1)]
        performance = performance[performance.sharpe >= performance.sharpe.quantile(0.9)]

        if maximize_returns:
            performance.sort_values(['average_returns', 'absolute_return_volatility', 'current_week_limit', 
                                     'next_week_limit', 'rtd_threshold_lower', 'rtd_threshold_upper', 'average_MDD'], 
                                    ascending = [False, True, False, False, False, True, False], inplace = True)

        else:
            performance.sort_values(['absolute_return_volatility', 'average_returns', 'current_week_limit', 
                                     'next_week_limit', 'rtd_threshold_lower', 'rtd_threshold_upper', 'average_MDD'], 
                                    ascending = [True, False, False, False, False, True, False], inplace = True)
        
        return performance.head(top_n).to_dict('records')
        '''
        {'current_week_limit': 1,
         'next_week_limit': 1,
         'rtd_threshold_lower': 0,
         'rtd_threshold_upper': 100,
         'average_returns': 0.045103822092706025,
         'absolute_return_volatility': 0.011403502790589215,
         'average_MDD': -0.23581863551409965,
         'sharpe': 3.955260319656179,
         'top_n'}
        '''

    def Hyperparameter_Tuner(self):
        
        self._get_profit_maximizing_hyperparams()
        
        simulation = self.Simulate()
        profit_maximizing_params = self._filter_best_micro_params(simulation, maximize_returns = self.simulation_parameters["maximize_returns"], top_n=self.simulation_parameters["top_n"])
        for i in range(len(profit_maximizing_params)):
            profit_maximizing_params[i]["date"] = self.parameters["prediction_date"]
            profit_maximizing_params[i]["priority"] = i
        
        print(client.Strategy.Profit_Maximizing_Params.delete_many({"date" : self.parameters["prediction_date"]}).deleted_count, f" records deleted Strategy.Profit_Maximizing_Params")
        print(len(client.Strategy.Profit_Maximizing_Params.insert_many(profit_maximizing_params).inserted_ids), f" records inserted for Strategy.Profit_Maximizing_Params")
    
    def _bypass_prediction(self, date):    
        return self.Live_Strategies[(self.Live_Strategies.date == date)&((self.Live_Strategies.prediction == 1))]
    
    def _convert_to_next_week(self, temp):
        
        current_to_next_map = {current_ : next_ for current_, next_ in zip(self.current_week_strategy_variants, self.next_week_strategy_variants)}
        
        current_week_daily_data = temp[temp.strategy_variant.isin(self.current_week_strategy_variants)]
        next_week_daily_data = temp[temp.strategy_variant.isin(self.next_week_strategy_variants)]
        
        current_week_daily_data.strategy_variant = current_week_daily_data.strategy_variant.map(current_to_next_map)
        current_week_daily_data = current_week_daily_data[~current_week_daily_data.strategy_variant.isin(next_week_daily_data.strategy_variant)]
        
        return pd.concat([current_week_daily_data, next_week_daily_data])
        
    def Predict(self, directive= {"ignore_filter" : False, "Convert_To_Next" : False}):
        
        self._get_profit_maximizing_params()
        date_list = np.array(sorted(self.Live_Strategies.date.unique()))
        date_list = list(date_list[date_list >= self.parameters['prediction_date']])

        filtered_strategies = []
        for date in date_list:
            
            print(f"Predicting For {date}")
            temp_strategies = None
            for i in range(len(self.profit_maximizing_params)):
                
                if directive["ignore_filter"]:
                    temp = self._bypass_prediction(date)
                    
                else:
                    temp = self._generate_prediction(date, 
                                                 self.profit_maximizing_params[i]['current_week_limit'], 
                                                 self.profit_maximizing_params[i]['next_week_limit'], 
                                                 self.profit_maximizing_params[i]['rtd_threshold_upper'], 
                                                 self.profit_maximizing_params[i]['rtd_threshold_lower'])
                
                if directive["Convert_To_Next"]:
                    temp = self._convert_to_next_week(temp)
                    
                if temp_strategies is None or len(temp_strategies) == 0:
                    print(f"Using Priority {i}")
                    temp_strategies = copy.deepcopy(temp)
                                    
            filtered_strategies.append(temp_strategies[["date", "underlying", "strategy_variant", "pos_param", "mov_param", "prof_param", "rtd"]])
        
        filtered_strategies = pd.concat(filtered_strategies)

        print(client.Live_Trading.Live_Strategies.delete_many({"date" :{"$in":date_list}}).deleted_count, f" records deleted from Live_Trading.Live_Strategies")
        if len(filtered_strategies) != 0:
            print(len(client.Live_Trading.Live_Strategies.insert_many(filtered_strategies.to_dict('records')).inserted_ids), f"records inserted into Live_Trading.Live_Strategies")
            print(filtered_strategies)