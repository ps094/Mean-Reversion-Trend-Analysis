class Directional_Signal_Generator:
    
    def __init__(self, strategy_type, strategy_version, previous_close, position_parameter, extreme_move_parameter,
                 abstinence_parameter, straight_abstinence_parameter, straight_reversal_parameter, directional_move_parameter,
                 initial_move_parameter,initial_time_parameter):

        self.strategy_type=strategy_type
        ##position_parameter: take position after 0.095% drop from previous peak, or 0.095% rise from previous trough.
        #Wait for 1m candle to form. Dont wait at exit. Next position taking to be judged from exit candle close.
        #A drop of 0.37% and above from previous peak while ascending implies a downward trend (1 min candle close)
        #An increase of 0.37% and above from previous trough while descending implies an upward trend (1 min candle close)
        ##abstinence_parameter: An straight move greater than 0.15%-0.5% in one minute candle from last. Dont take any position.
        #After 0.15%-0.5% wait till the index move by 0.04% in the reverse before counting 0.95% candle from 0.15%-0.5% candle end.
        #if the difference between last trough and crest is greater tha 0.37% then it implies direction change
        
        self.strategy_version=strategy_version
        #[1:reversal is critical point]
        #[2:previous peak/trough is crtical point]
        
        self.closing_prices=[previous_close]
        self.critical_point=previous_close
        self.critical_index=0
        self.close=previous_close

        self.direction=0
        self.signal=0
        self.current_batch_id=0
        
        self.position_parameter=position_parameter
        self.extreme_move_parameter=extreme_move_parameter
        self.abstinence_parameter=abstinence_parameter
        self.straight_abstinence_parameter=straight_abstinence_parameter
        self.straight_reversal_parameter=straight_reversal_parameter
        self.directional_move_parameter=directional_move_parameter
        self.initial_move_parameter=initial_move_parameter
        self.initial_time_parameter=initial_time_parameter
        
        self.position_flag=0
        self.extreme_move_flag=0
        self.straight_move_flag=False
        self.initial_move_flag=0
                
    def _position_flag(self):
        
        percent_change=abs(100*(self.close-self.critical_point)/self.critical_point)
        
        if percent_change<(self.position_parameter):
            self.position_flag=0
        
        elif percent_change<(self.abstinence_parameter):
            self.position_flag=1
                    
        else:
            self.position_flag=0
    
    def _extreme_move_flag(self):

        percent_change=abs(100*(self.close-self.critical_point)/self.critical_point)
        
        if percent_change<self.extreme_move_parameter:
            self.extreme_move_flag=0
        else:
            self.extreme_move_flag=1
            
    def _up_move_indicator(self,subset):
        
        if len(subset)>1:
            
            index=1
            lower=subset[0]
            upper=subset[0]
            upper_index=0
            straight_flag=False
            
            abstinence=self.straight_abstinence_parameter*self.critical_point/100
            reversal=self.straight_reversal_parameter*self.critical_point/100

            while index<len(subset):

                if subset[index]>=upper:
                    upper=subset[index]
                    upper_index=index
                    
                elif (upper-subset[index])>=reversal:
                    
                    if straight_flag==True:
                        straight_flag=False
                        self.critical_point=subset[index]
                        self.critical_index+=index
                            
                    lower=subset[index]
                    upper=subset[index]
                    upper_index=index
                        
                if straight_flag==False and (upper-lower)>=abstinence:
                    straight_flag=True

                index+=1
            
            return straight_flag
        
        else:
            return False

    def _down_move_indicator(self,subset):
        
        if len(subset)>1:

            index=1
            lower=subset[0]
            upper=subset[0]
            lower_index=0
            straight_flag=False
            
            abstinence=self.straight_abstinence_parameter*self.critical_point/100
            reversal=self.straight_reversal_parameter*self.critical_point/100

            while index<len(subset):

                if subset[index]<=lower:
                    lower=subset[index]
                    lower_index=index

                elif (subset[index]-lower)>=reversal:

                    if straight_flag==True:
                        
                        straight_flag=False
                        self.critical_point=subset[index]
                        self.critical_index+=index

                    lower=subset[index]
                    upper=subset[index]
                    lower_index=index

                if straight_flag==False and (upper-lower)>=abstinence:
                    straight_flag=True

                index+=1
            
            return straight_flag
        
        else:
            return False
        
    def _straight_move_flag(self):
        
        up_signal=self._up_move_indicator(self.closing_prices[self.critical_index:])
        if up_signal==True:
            self.straight_move_flag=True
        
        else:
            down_signal=self._down_move_indicator(self.closing_prices[self.critical_index:])
            
            if down_signal==True:
                self.straight_move_flag=True
            
            else:
                self.straight_move_flag=False
    
    def _direction_flag(self):
        
        subset=self.closing_prices[1:]
        direction=0
        initialmovequanta=(self.initial_move_parameter*self.closing_prices[0])/100
        
        if len(subset)>0:
            
            upper=subset[0]
            lower=subset[0]
            index=1
            directionalmovequanta=(self.directional_move_parameter*self.closing_prices[0])/100
            
            while index < len(subset):

                if subset[index]>upper:
                    upper=subset[index]

                elif subset[index]<lower:
                    lower=subset[index]

                if subset[index]-lower >= directionalmovequanta:
                    direction=1
                    lower=subset[index]

                elif upper - subset[index] >= directionalmovequanta:
                    direction=-1
                    upper=subset[index]
                
                index+=1
        
        if direction!=0:
            
            self.direction=direction
        
        elif self.close-self.closing_prices[0] > initialmovequanta:
                
            self.direction=1
            self.initial_move_flag=1
            
        elif self.closing_prices[0]-self.close > initialmovequanta:
        
            self.direction=-1
            self.initial_move_flag=-1
        
        else:
            
            self.direction = self.initial_move_flag
    
    def Update(self,batch_id,price,critical=False):
        
        if batch_id>self.current_batch_id:
            
            self.current_batch_id=batch_id
            self.closing_prices.append(price)
            self.close=price
            
            if critical:
                self.critical_point=price
                self.critical_index=batch_id
            
            self._direction_flag()
            self._straight_move_flag()
            self._extreme_move_flag()
            self._position_flag()
                            
            if self.direction==0:
                self.signal=0
            
            elif self.extreme_move_flag==1:
                self.signal=1

            elif self.straight_move_flag==False:

                if self.position_flag==1:
                    self.signal=1
                else:
                    self.signal=0
            
            else:
                self.signal=0
            
            if self.strategy_version==2:
                self.direction*=-1
                
            if self.current_batch_id>=self.initial_time_parameter:
                return self.direction, self.signal
            else:
                return 0,0

        else:
            print("Same Batch ID Provided To Signal Generator")
            return 0, 0 