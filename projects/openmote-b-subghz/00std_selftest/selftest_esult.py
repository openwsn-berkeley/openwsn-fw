import paho.mqtt.client as mqtt
import json
import base64
import time
import sys, getopt
import Queue
import random
import struct

#============================ defines =========================================
BROKER_ADDRESS        = "argus.paris.inria.fr"

#============================ class ===========================================
class mqtt_client(object):

    def __init__(self):
        self.result = {}
        
    #==== mqtt admin

    def connect_to_mqtt(self):
    
        mqttclient                = mqtt.Client('selftest')
        mqttclient.on_connect     = self._on_mqtt_connect
        mqttclient.on_message     = self._on_mqtt_message
        mqttclient.connect(BROKER_ADDRESS)
        mqttclient.loop_start()
        
        # wait for a while to gather the response from otboxes
        time.sleep(10)
        
        # close the client and return the motes list
        mqttclient.loop_stop()
        
        with open("result.txt".format(time.time()),'w') as f:
            for item, value in self.result.items():
                f.write("{0}: {1}".format(item,value))
                f.write('\n')
    
    def _on_mqtt_connect(self, client, userdata, flags, rc):
        
        print "subscribe to opentestbed/deviceType/mote/deviceId/+/notif/frommoteserialbytes"
        client.subscribe('opentestbed/deviceType/mote/deviceId/+/notif/frommoteserialbytes')
       
            
    def _on_mqtt_message(self, client, userdata, message):
        
        payload = json.loads(message.payload)
        
        try:
            self.result.update({message.topic.split('/')[4]: payload['serialbytes'][:10]})
        except:
            print "something wrong"
            
#============================ main function ===================================
mqtt_client().connect_to_mqtt()
raw_input('press any key to return')
    
    
    
    
    
    
    
    
    
    
    