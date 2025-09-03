import tensorflow as tf
import pandas as pd
import numpy as np
from sklearn.metrics import precision_score
from tensorflow.keras import regularizers
from keras.layers import Dropout
import warnings
warnings.filterwarnings("ignore")

def ann_model(params):
    
    ann = tf.keras.models.Sequential()
    #num_layers, 
    # num_cells,
    #kernel_regularizer_l1,
    #kernel_regularizer_l2, 
    #bias_regularizer, 
    #activity_regularizer, 
    #dropout, 
    #activation, 
    #learning_rate, 
    #beta_1
    #epochs
    
    for i in range(params['num_layer']):

        ann.add(tf.keras.layers.Dense(units = params['num_cell'], activation = params['activation'], \
                                      kernel_regularizer=regularizers.L1L2(l1 = params['kernel_regularizer_l1'], \
                                                                           l2 = params["kernel_regularizer_l2"]), \
                                      bias_regularizer = regularizers.L2(params['bias_regularizer']), \
                                      activity_regularizer = regularizers.L2(params['activity_regularizer'])))
        ann.add(Dropout(params['dropout']))
       
    ann.add(tf.keras.layers.Dense(units = 1, activation = 'sigmoid'))
    ann.compile(optimizer = tf.keras.optimizers.legacy.Adam(learning_rate = params['learning_rate'], beta_1 = params['beta']),\
                loss = 'binary_crossentropy', metrics = ['accuracy'])
    
    return ann

def run_sim(params):
    
    #'X_train' , 'y_train', X_test', 'y_test', 'identifier'
    
    try:
            
        model = ann_model(params)
        model.fit(params['X_train'], params['y_train'], batch_size = 32, epochs = params['epoch'], verbose = 0, \
              class_weight = {0: params['class_weight'], 1: 1})
    
        y_pred = model.predict(params['X_test'], verbose=0)
        y_pred = (y_pred > params['precision_filter'])
    
    except Exception as e:
     
        print(e, ". Error in running simulation using ANN ")
        y_pred = np.array([[0] for i in range(len(params['X_test']))])
        
    return params['identifier'], y_pred