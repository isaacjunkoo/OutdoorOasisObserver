from flask import Flask, request

app = Flask(__name__)

from openai import OpenAI
global client
import requests
client = OpenAI(api_key='secret')
import ast



def get_data():
    #get plant data and plant name
    print(request.args.get("var"))
    return "We received value: "+str(request.args.get("var"))


def get_chat(temp, humidity, visible, soil_moisture, plant_name):
    # temp, humidity, visible, soil_moisture = get_data()

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



@app.route('/')
def home():
    return HOME_HTML

HOME_HTML = """
    <body style="background-color:MediumSeaGreen;">
    <html><body>
        <h1 style="font-family:'Brush Script MT', cursive;font-size:400%;text-align:center;">Welcome to the OOO<h1>
        <form action="/greet" style=text-align:center; >
            What plant are you trying to monitor? <br> <br> <input style= "width:500px"; type='text' name='plant'><br> <br>
            <input style= "width:500px"; type='submit' value='Continue'>
        </form>
    </body></html>"""

def greet():
    plant = request.args.get('plant', '')
    # get current measurement from sensors
    temp, humidity, moisture, sunlight = get_data()
    # get ideal measurement from apis
    params, recommendations = get_chat(temp, humidity, sunlight, moisture, plant)
    itemp = params[0]
    ihumidity = params[1]
    isunlight = params[2]
    imoisture = params[3]

    if plant == '':
        msg1 = 'You did not tell me the name of the plant.'
    else:
        msg1 = 'The plant that we will be monitoring is ' + plant + '. <br>'
        msg2 = (' Current Humidity: ' + str(humidity) + ' Ideal Humidity: ' + str(ihumidity) + '<br>' +
                ' Current Sun Light Level: ' + str(sunlight) + ' Ideal Sun Light Level: ' + str(isunlight) + '<br>' +
                ' Current Tempertaure: ' + str(temp) + ' Ideal Temperature: ' + str(itemp) + '<br>' +
                ' Current Moisture ' + str(moisture) + ' Ideal Moisture: ' + str(imoisture) + '<br>')

    return RESULT_HTML.format(plant, msg1, msg2, humidity, sunlight, temp, moisture, ihumidity, isunlight, itemp,
                              imoisture, recommendations)


RESULT_HTML = """
    <body style="background-color:MediumSeaGreen;">
    <html>
    <script src="https://www.gstatic.com/charts/loader.js"></script>
    <body>
        <h1 style="font-family:'Brush Script MT', cursive;font-size:400%;text-align:center;"> {0}</h1>
        <p style="font-family:courier;text-align:center;"> {1}</p>
        <p style="font-family:courier;text-align:center;"> {2}</p>

        <script>
        google.charts.load('current', {{'packages':['corechart']}});
        google.charts.setOnLoadCallback(drawHumidityChart);
        google.charts.setOnLoadCallback(drawSunlightChart);
        google.charts.setOnLoadCallback(drawTempChart);
        google.charts.setOnLoadCallback(drawMoistureChart);

        function drawHumidityChart() {{

        // Set Data
        const data = google.visualization.arrayToDataTable([
        ['Humidity', 'g/kg'],
        ['Current',{3}],
        ['Ideal', {7}],
        ]);

        // Set Options
        const options = {{
            title:'Current vs Ideal Humidity',
            width:400,
            height:300
        }};

        // Draw
        const chart = new google.visualization.BarChart(document.getElementById('humidityChart'));
        chart.draw(data, options);

        }}

        function drawSunlightChart() {{

        // Set Data
        const data = google.visualization.arrayToDataTable([
        ['Sun Light Level', 'W/m2'],
        ['Current',{4}],
        ['Ideal', {8}],
        ]);

        // Set Options
        const options = {{
            title:'Current vs Ideal Sun Light Level',
            width:400,
            height:300
        }};

        // Draw
        const chart = new google.visualization.BarChart(document.getElementById('sunlightChart'));
        chart.draw(data, options);

        }}

        function drawTempChart() {{

        // Set Data
        const data = google.visualization.arrayToDataTable([
        ['Temp', '°F'],
        ['Current',{5}],
        ['Ideal', {9}],
        ]);

        // Set Options
        const options = {{
            title:'Current vs Ideal Temperature',
            width:400,
            height:300
        }};

        // Draw
        const chart = new google.visualization.BarChart(document.getElementById('temperatureChart'));
        chart.draw(data, options);

        }}

        function drawMoistureChart() {{

        // Set Data
        const data = google.visualization.arrayToDataTable([
        ['Moisture', '°F'],
        ['Current',{6}],
        ['Ideal', {10}],
        ]);

        // Set Options
        const options = {{
            title:'Current vs Ideal Moisture',
            width:400,
            height:300
        }};

        // Draw
        const chart = new google.visualization.BarChart(document.getElementById('moistureChart'));
        chart.draw(data, options);

        }}
        </script>

        <table class="columns">
      <tr>
        <td><div id="humidityChart" style="border: 1px solid #ccc"></div></td>
        <td><div id="sunlightChart" style="border: 1px solid #ccc"></div></td>
        <td><div id="temperatureChart" style="border: 1px solid #ccc"></div></td>
        <td><div id="moistureChart" style="border: 1px solid #ccc"></div></td>
      </tr>
    </table>
    <p style="font-family:courier;text-align:center;"> Recommendations: </p>
    <p style="font-family:courier;text-align:center;"> {11}</p>
    </body></html>
    """

if __name__ == "__main__":
    # Launch the Flask dev server
    app.run(host="localhost", debug=True)
