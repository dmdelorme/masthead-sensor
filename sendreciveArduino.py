import RPi.GPIO as GPIO
from lib_nrf24 import NRF24
import time
import spidev
def isfloat(value):
  try:
    float(value)
    return True
  except ValueError:
    return False
GPIO.setmode(GPIO.BCM)
INPUT_PIN = 25           # Sets our input pin, in this example I'm connecting our button to pin 25. Pin 0 is the SDA pin so I avoid using it for sensors/buttons
GPIO.setup(INPUT_PIN, GPIO.IN)           # Set our input pin to be an input

pipes = [[0xE8, 0xE8, 0xF0, 0xF0, 0xE1], [0xF0, 0xF0, 0xF0, 0xF0, 0xE1]]

radio = NRF24(GPIO, spidev.SpiDev())
radio.begin(0, 17)

radio.setPayloadSize(32)
radio.setChannel(0x76)
radio.setDataRate(NRF24.BR_1MBPS)
radio.setPALevel(NRF24.PA_MIN)

radio.setAutoAck(True)
radio.enableDynamicPayloads()
radio.enableAckPayload()

radio.openWritingPipe(pipes[0])
radio.openReadingPipe(1, pipes[1])
radio.printDetails()
radio.startListening()
message = list("G3")
while len(message) < 2:
    message.append(0)

while True:
    start = time.time()
    if (GPIO.input(INPUT_PIN) == True): # Physically read the pin now
      message = list("G1")
      while len(message) < 2:
          message.append(0)
    else:
      message = list("G0")
      while len(message) < 2:
          message.append(0)

    radio.write(message)
    #print("Sent the message: {}".format(message))
    radio.startListening()
    
    while not radio.available(0):
        time.sleep(3/100)
        if time.time() - start > 2:
            print("Timed out.")
            break
    
    receivedMessage = []
    radio.read(receivedMessage, radio.getDynamicPayloadSize())
    #print("Received: {}".format(receivedMessage))

    #print("Translating our received Message into unicode characters...")
    string = ""

    for n in receivedMessage:
        if (n >= 32 and n <= 126):
            string += chr(n)
    #print("Our received message decodes to: {}".format(string))
    aws = 0.0
    awa = 0.0
    string = string.strip()
    if string:
      a, b = string.split(",")
      if isfloat(a):
        aws = float(a)
        awa = round(float(b) * 0.0174533,5)
        #print repr (aws) +', '+ repr(awa)
      else:
        aws = string
    import socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    SignalK = '{"updates": [{"source": {"type": "wind","src" : "arduino"},"values":['
    SignalK += '{"path": "environment.wind.speedApparent","value":'+str(aws)+'}'
    SignalK += ',{"path": "environment.wind.angleApparent","value":'+str(awa)+'}'
    SignalK += ']}]}\n'
    #print (SignalK)
    sock.sendto(SignalK, ('localhost', 55559))
    sock.close()
    radio.stopListening()
    time.sleep(.5)
