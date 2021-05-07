import os
import json
import time
import requests


TIME=45
DEVICE_ID = os.environ.get('DEVICE_ID')
JWT_TOKEN = os.environ.get('JWT_TOKEN')
BASE = 'https://thingsboard.e-yantra.org/api'


if __name__ == "__main__":
    with open('rpc.json') as fp:
        rpc_list = json.load(fp)
    for req in rpc_list:
        try:
            response = requests.post(f'{BASE}/plugins/rpc/oneway/{DEVICE_ID}', 
                json=req,
                headers={
                    'X-Authorization': f'Bearer {JWT_TOKEN}',
                })
            if response.status_code // 100 == 2:
            	print(f'{req} successfully sent.')
            else:
            	print(f'Failed {response.json()}')
        except requests.exceptions.RequestException as e:
            # print(response.json())
            print(f'Problem in sending request: {req}')
            print(e)
        time.sleep(TIME)
