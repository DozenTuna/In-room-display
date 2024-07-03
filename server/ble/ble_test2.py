import bluepy
from config import bt_addr, bt_addr_mac

peri = bluepy.btle.Peripheral()
peri.connect(bt_addr,bluepy.btle.ADDR_TYPE_PUBLIC)
print("connected")
peri.disconnect()

