import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import time

def _evaluate_scalping_on_path(params):
    
    try:
        mod_count_limit = params["mod_count_limit"]
        price_trail = params["price_trail"]
        min_price_change_multiplier = params["min_price_change_multiplier"]
        path = params["path"]
        option_price = params["option_price"]
        delta = params["delta"]
        S0 = params["S0"]
        
        upper_mod_count = 0
        lower_mod_count = 0
        upper_flag = False
        lower_flag = False
        upper_bound = S0 * (1 + price_trail / 10000)
        lower_bound = S0 * (1 - price_trail / 10000)
        upper_distance = 0
        lower_distance = 0
        upper = None
        lower = None
        upper_target = None
        lower_target = None
        profit = None
        trade_duration = 0

        for step in path:

            trade_duration+=1

            if upper_mod_count < mod_count_limit:        
                upper_target = step * (1 + price_trail / 10000)
                upper_distance = (upper_bound - upper_target) / upper_bound - price_trail * min_price_change_multiplier / 10000
            elif upper_mod_count == mod_count_limit:
                upper_target = step
                upper_distance = (upper_bound - upper_target) / upper_bound - price_trail * min_price_change_multiplier / 10000
            else:
                upper_target = step
                upper_distance = (upper_bound - upper_target) / upper_bound


            if lower_mod_count < mod_count_limit:        
                lower_target = step * (1 - price_trail / 10000)
                lower_distance = (lower_target - lower_bound) / lower_bound - price_trail * min_price_change_multiplier / 10000
            elif lower_mod_count == mod_count_limit:
                lower_target = step
                lower_distance = (lower_target - lower_bound) / lower_bound - price_trail * min_price_change_multiplier / 10000
            else:        
                lower_target = step
                lower_distance = (lower_target - lower_bound) / lower_bound


            if not upper_flag and upper_distance >= 0:
                upper_bound = upper_target
                upper_mod_count += 1

            if not lower_flag and lower_distance >= 0:
                lower_bound = lower_target
                lower_mod_count += 1


            if step >= upper_bound and not upper_flag:
                upper_flag = True
                upper = upper_bound

            if step <= lower_bound and not lower_flag:
                lower_flag = True
                lower = lower_bound

            if upper_flag and lower_flag:
                break

        if not upper_flag:
            upper = path[-1]

        if not lower_flag:
            lower = path[-1]

        return (upper - lower) * delta * 0.5 / option_price - 0.001, trade_duration / 60
    
    except Exception as e:
        print(f"{e}, Trade Sim error for {params}", flush=True)
        return 0, 0
