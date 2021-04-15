import os
import json
import time
import requests


TIME=45
DEVICE_ID = "9dd9bca0-809f-11eb-950e-efef5c07c810"
JWT_TOKEN = ""
BASE = 'https://thingsboard.e-yantra.org/api'


if __name__ == "__main__":
    response = requests.post(f'{BASE}/auth/login', json={
        'username': "kachraseth@cs684.edu",
        'password': "150rupyadega"
    })
    response_json = response.json()
    JWT_TOKEN = response_json["token"]
    with open('rpc.json') as fp:
        rpc_list = json.load(fp)
    for req in rpc_list:
        try:
            response = requests.post(f'{BASE}/plugins/rpc/oneway/{DEVICE_ID}', 
                json=req,
                headers={
                    'X-Authorization': f'Bearer {JWT_TOKEN}',
                })
            print(response)
            if response.status_code // 100 == 2:
            	print(f'{req} successfully sent.')
            else:
            	print(f'Failed {response.json()}')
        except requests.exceptions.RequestException as e:
            # print(response.json())
            print(f'Problem in sending request: {req}')
            print(e)
        time.sleep(TIME)

