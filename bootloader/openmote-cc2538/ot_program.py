import paho.mqtt.client as mqtt
import json
import base64
import time
import sys, getopt
import Queue
import random
import os

#============================ defines =========================================
BROKER_ADDRESS          = "argus.paris.inria.fr"
NUMBER_OF_MOTES         = 80 - 4 # 4 motes are used for local test

#============================ classes =========================================
class program_over_testbed(object):
    
    CLIENT_ID               = "OpenWSN"
    CMD                     = 'program'
    
    # in seconds, should be larger than the time starting from publishing message until receiving the response
    MESSAGE_RESP_TIMEOUT    = 30
    
    def __init__(self, mote, image_path):
        
        # initialize parameters
        self.mote       = mote
        self.image      = None
        
        # check bootload backdoor is configured correctly
        
        bootloader_backdoor_enabled   = False
        extended_linear_address_found = False
        
        with open(image_path,'r') as f:
            for line in f:
            
                # looking for data at address 0027FFD4
                # refer to: https://en.wikipedia.org/wiki/Intel_HEX#Record_types
                
                # looking for upper 16bit address 0027
                if line[:15] == ':020000040027D3':
                    extended_linear_address_found = True
            
                # check the lower 16bit address FFD4
                
                # | 1:3 byte count | 3:7 address | 9:17 32-bit field of the lock bit page (the last byte is backdoor configuration) |
                # 'F6' = 111        1                               0           110
                #        reserved   backdoor and bootloader enable  active low  PA pin used for backdoor enabling (PA6)
                if extended_linear_address_found and line[3:7] == 'FFD4' and int(line[1:3], 16)>4  and line[9:17] == 'FFFFFFF6':
                    bootloader_backdoor_enabled = True
        
        assert bootloader_backdoor_enabled
        
        self.image_name = ''
        with open(image_path,'rb') as f:
            self.image = base64.b64encode(f.read())
        if os.name=='nt':       # Windows
            self.image_name = image_path.split('\\')[-1]
        elif os.name=='posix':  # Linux
            self.image_name = image_path.split('/')[-1]
        
        # initialize statistic result
        self.response_success = {
            'success_counter':   0 ,
            'message_counter':   0 ,
            'failed_messages_topic': []
        }
        
        # mqtt topic string format
        self.mqtttopic_mote_cmd     = 'opentestbed/deviceType/mote/deviceId/{0}/cmd/{1}'.format(self.mote,self.CMD)
        self.mqtttopic_mote_resp    = 'opentestbed/deviceType/mote/deviceId/{0}/resp/{1}'.format(self.mote,self.CMD)
        
        # connect to MQTT
        self.mqttclient                = mqtt.Client(self.CLIENT_ID)
        self.mqttclient.on_connect     = self._on_mqtt_connect
        self.mqttclient.on_message     = self._on_mqtt_message
        self.mqttclient.connect(BROKER_ADDRESS)
        self.mqttclient.loop_start()

        # create queue for receiving resp messages
        self.cmd_response_success_queue        = Queue.Queue()
        
        payload_program_image = {
            'token':       123,
            'description': self.image_name,
            'hex':         self.image,
        }
        # publish the cmd message
        self.mqttclient.publish(
            topic   = self.mqtttopic_mote_cmd,
            payload = json.dumps(payload_program_image),
        )
        
        try:
            # wait maxmium MESSAGE_RESP_TIMEOUT seconds before return
            self.cmd_response_success_queue.get(timeout=self.MESSAGE_RESP_TIMEOUT)
        except Queue.Empty as error:
            print "Getting Response messages timeout in {0} seconds".format(self.MESSAGE_RESP_TIMEOUT)
        finally:
            self.is_response_success()
            self.mqttclient.loop_stop()

    #======================== private =========================================
    def _on_mqtt_connect(self, client, userdata, flags, rc):

        # subscribe to box commands
        if self.mote == 'all':
            topic = 'opentestbed/deviceType/mote/deviceId/{0}/resp/{1}'.format('+',self.CMD)
        else:
            topic = self.mqtttopic_mote_resp

        client.subscribe(topic)
        # print "subscribe at {0}".format(topic)
        
        client.loop_start()
    
    def _on_mqtt_message(self, client, userdata, message):
        '''
        Record the number of message received and success status
        '''
        
        self.response_success['message_counter'] += 1
        if json.loads(message.payload)['success']:
            self.response_success['success_counter'] += 1
        else:
            self.response_success['failed_messages_topic'].append(message.topic)
        
        if self.mote == 'all':
            if self.response_success['message_counter'] == NUMBER_OF_MOTES:
                self.cmd_response_success_queue.put('unblock')
        else:
            self.cmd_response_success_queue.put('unblock')
        
    #======================== public ==========================================
    def is_response_success(self):
        print "--------------------------------------------------------------"
        print "Try to program {0} motes, {1} motes report with success".format(
            self.response_success['message_counter'],
            self.response_success['success_counter']
        )
        if self.response_success['message_counter'] > self.response_success['success_counter']:
            print "failed_messages_topic :"
            for topic in self.response_success['failed_messages_topic']:
                print "    {0}".format(topic)
        print "--------------------------------------------------------------"

#============================ helper ==========================================
def usage():
    print("""Usage: %s [-h]  [-b board] [-a mote eui64 address] [file.ihex]
    -h, --help               This help
    -b board                 Board name (right now only support openmote-b)
    -a mote address          Mote address in eui64 format(00-12-4b-00-14-b5-b4-98) or use "all" indicating all motes

    Examples:
        .python opentestbed_reprogram.py -b openmote-b -a 00-12-4b-00-14-b5-b4-98 example/main.ihex
        .python opentestbed_reprogram.py -b openmote-b -a all example/main.ihex
    """)
       

#============================ main ============================================
if __name__ == "__main__":

    configure = {}

    #==== get the options
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hb:a:", ['help', 'board=', 'mote_address='])
    except getopt.GetoptError as err:
        print(str(err))
        usage()
        sys.exit(2)

    for opt, arg in opts:
        if opt == '-h' or opt == '--help':
            usage()
            sys.exit(0)
        elif opt == '-b' or opt == '--board':
            configure['board'] = arg
        elif opt == '-a' or opt == '--mote_address':
            configure['mote_address'] = str(arg)
        else:
            assert False, "Unhandled option"

    try:
        configure['image_name_path'] = args[0]
    except:
        raise Exception('No file path given.')
    
    #==== program_over_testbed
    program_over_testbed(configure['mote_address'], configure['image_name_path'])