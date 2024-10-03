import json
import boto3
import decimal
import time

def lambda_handler(event, context):
    dynamodb = boto3.resource('dynamodb')
    # 指定DynamoDB表名稱
    table_name = 'map'
    
    # 獲取DynamoDB表對象
    table = dynamodb.Table(table_name)
    respond = table.scan()

    #接收GPS模組request
    if event['key'] == "location" :
        lat = event['lat']
        lon = event['lon']
        
        #轉換為 Decimal 物件
        lat_decimal = decimal.Decimal(str(lat))
        lon_decimal = decimal.Decimal(str(lon))
        current_time_integer = int(time.time())
   
        # 將資訊寫入DynamoDB
        response = table.put_item(
            Item={
                'id': current_time_integer,
                'lat': lat_decimal,
                'lon': lon_decimal
            }   
        )
        #接收網站request
    else :
        column_name = 'id'
        max_value = None
        response = table.scan(ProjectionExpression=column_name)
        for item in response['Items']:
            value = item.get(column_name)
            if value is not None:
                #找出'id'欄位的最大值
                if max_value is None or value > max_value:
                    max_value = value
        response = table.get_item(
            Key={
                'id': max_value
            }
        )
        item = response.get('Item')
        if item:
            # 獲取浮點數值
            lat_ = float(item.get('lat'))
            lon_ = float(item.get('lon'))
            # 返回經緯度值
            return {
                'statusCode': 200,
                'lat': lat_,
                'lon': lon_
            }
        else:
            return {
                'statusCode': 404,
                'body': 'Item not found'
            }
