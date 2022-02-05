import urllib.request
import json

resp = urllib.request.urlopen("https://d1jithvltpp1l1.cloudfront.net/getLiveURL?channelno=332&mode=prod&audioCode=&format=HLS&callerReferenceNo=20140702122500").read().decode("utf-8")
body = json.loads(resp)
print(body["asset"]["hls"]["adaptive"][0])
