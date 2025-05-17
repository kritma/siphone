SID=""
PASS=""
curl -X POST https://api.twilio.com/2010-04-01/Accounts/$SID/Tokens.json -u $SID:$PASS > ice_servers.json
