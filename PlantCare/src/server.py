from openai import OpenAI
global client
import requests
from flask import request
client = OpenAI(api_key='sk-proj-ZC1toUDKGV726DLPTfq5T3BlbkFJtmYQXfjsMo9gl04yC8jL')
import ast




def get_data():
    #get plant data and plant name
    print(request.args.get("var"))
    return "We received value: "+str(request.args.get("var"))


def get_chat():
    temp, humidity, visible, soil_moisture, plant_name = get_data()

    #weather API
    city_name = "Irvine"
    country = "US"
    lat, lon = get_LL(city_name, country)
    temps, UVs, precips = get_weather(lat, lon)
    print(temps)
    print(UVs)
    print(precips)

    #weather API


    prompt_p = f"I am growing a {plant_name}. The current conditions are the following: Temperature : {temp}, Humidity : {humidity}, Visible Light Level : {visible}, Soil Moisture Level: {soil_moisture} lux. Please give me optimal growth conditions in the format of : [Optimal Temperature, Optimal Humidity, Optimal Visible Light Level (lux), Optimal Soil Moisture Level %]. No other words or symbols in the response is wanted, only 1 number per category and keep responses in the brackets []."


    responses = client.chat.completions.create(
        model="gpt-3.5-turbo",
        messages=[{"role": "user", "content": prompt_p}]
    )

    recommendations = responses.choices[0].message.content

    print("NUMBERS!!!\n")

    actual_list = ast.literal_eval(recommendations)
    print(actual_list)


    prompt_p = f"I am growing a {plant_name}. The current conditions are the following: Temperature : {temp}, Humidity : {humidity}, Visible Light Level : {visible}, Soil Moisture Level: {soil_moisture}. The forecast for this week is: High Temps: {str(temps)}, Max UV Readings Per Day: {str(UVs)}, Precipitation: {str(precips)}. Give me specific list of actions to raise the plants more optimally taking into consideration to the given conditions and the weather predictions. The response should use the weather predictions for the week in its answers"

    responses = client.chat.completions.create(
        model="gpt-3.5-turbo",
        messages=[{"role": "user", "content": prompt_p}]
    )

    recommendations = responses.choices[0].message.content

    print("RECOMMDENDATIONS!!!\n")
    print(recommendations)

    return actual_list, recommendations



def get_LL(city, country):

    api_url = 'https://api.api-ninjas.com/v1/geocoding?city=' + \
        city + "&country=" + country
    try:
        response = requests.get(
            api_url + city, headers={'X-Api-Key': 'zxyVXo+dx/UeCEQ/z+scTw==lefzzJ7tEE2q9aYn'}).json()
        print(response[0])
        return response[0]['latitude'], response[0]['longitude']
    except:
        print("Invalid Location or Failed Repsonse")


def get_weather(lat, lon):

    temps = []
    UVs = []
    precips = []
    
    url = "https://api.open-meteo.com/v1/forecast?latitude=" + str(lat) + "&longitude=" + str(lon) + \
    "&daily=temperature_2m_max,uv_index_max,precipitation_sum&temperature_unit=fahrenheit&timezone=auto"
    response = requests.get(url).json()
    for k, v in response['daily'].items():

        separated = [v[i:i+24] for i in range(0, len(v), 24)]

        if k == "temperature_2m_max":
            temps = [round(x) for x in separated[0]]
        elif k == 'uv_index_max':
            UVs = separated[0]

        elif k == "precipitation_sum":
            precips = separated[0]
        

    return temps, UVs, precips


if __name__ == "__main__":
    print(get_chat())




