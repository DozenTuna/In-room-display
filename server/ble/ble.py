from bluepy.btle import Scanner
import subprocess
import requests
from config import line_token, bt_addr


def send_line(msg):
    #サーバーに送るパラメータを用意
    url = 'https://notify-api.line.me/api/notify'
    headers = {'Authorization': 'Bearer ' + line_token}
    payload = {'message': msg}
    #requestsモジュールのpost関数を利用してメッセージを送信する
    #ヘッダにトークン情報，パラメータにメッセージを指定する
    requests.post(url, headers=headers, params=payload)

scanner = Scanner()
try:
    while True:
        print("scanning...")
        try:
            devices = scanner.scan(30.0)
        except Exception as e:
            print(e," has occured")
            scanner = Scanner()
            devices = scanner.scan(30.0)
        status = "OUT"
        for dev in devices:
            #print(dev.addr,dev.addrType)
            if dev.addr == bt_addr:
                #print("Device %s (%s), RSSI=%d dB" % (dev.addr, dev.addrType, dev.rssi))
                try:
                    TxPower = int(dev.getValueText(0x0a),16)
                    RSSI = dev.rssi
                    d=pow(10,((TxPower - RSSI - 41)/20))
                    print("Distance=",d)
                    if d < 100:
                        status = "IN"
                    else:
                        status = "NEAR"
                except:
                    status = previous_status
        print("status = ",status)
        command = ['ln', '-sf', '/home/s.kazuki/src/http/img/sakamoto_in.png', '/home/s.kazuki/src/http/img/sakamoto.png']
        if status == "IN":
            pass
        elif status == "NEAR":
            command[2] = '/home/s.kazuki/src/http/img/sakamoto_near.png'
        else:
            command[2] = '/home/s.kazuki/src/http/img/sakamoto_out.png'
        subprocess.run(command)
        previous_status = status
except Exception as err:
    print("Error occurd:",err)
    message_for_line = "Error occurd: " + err
    send_line(message_for_line)
    command = ['ln', '-sf', '/home/s.kazuki/src/http/img/sakamoto_error.png', '/home/s.kazuki/src/http/img/sakamoto.png']
    subprocess.run(command)


