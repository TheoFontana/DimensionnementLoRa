from decouple import config
from ipywidgets.embed import embed_minimal_html
import pandas as pd
import gmaps
import numpy as np
import matplotlib.pyplot as plt

# Put the value of your Google API key inside a .env file 
gmaps.configure(api_key=config('GOOGLE_API_KEY'))

# Coordonates of key points
GEI_coordinate = (43.570819, 1.466246) 
LoRa_antenas = [(43.570819, 1.466246)]

def min_max_scaling(series):
    return (series - series.min()) / (series.max() - series.min())

# Python 3 program to calculate Distance Between Two Points on Earth
from math import radians, cos, sin, asin, sqrt
def distance(lat1, lat2, lon1, lon2):
     
    # The math module contains a function named
    # radians which converts from degrees to radians.
    lon1 = radians(lon1)
    lon2 = radians(lon2)
    lat1 = radians(lat1)
    lat2 = radians(lat2)
      
    # Haversine formula
    dlon = lon2 - lon1
    dlat = lat2 - lat1
    a = sin(dlat / 2)**2 + cos(lat1) * cos(lat2) * sin(dlon / 2)**2
 
    c = 2 * asin(sqrt(a))
    
    # Radius of earth in meters . Use 3956 for miles
    r = 6371000
      
    # calculate the result
    return(c * r)

def create_distance(lattitude, longitude,GEI_coordinate):
    dist = []
    n =len(lattitude)
    for i in range(n) :
        lat=lattitude[i]
        long=longitude[i]
        dist.append(distance(lat, GEI_coordinate[0], long,GEI_coordinate[1]))
    return dist

data = pd.read_csv(r'data.csv')
data.insert(0, "dist", create_distance(data['lattitude'], data['longitude'], GEI_coordinate))


data['rssiPacket'] = min_max_scaling(data['rssiPacket'])

locations = data[['lattitude', 'longitude']]
weights = data['rssiPacket']


# Create the google maps figure
figure_layout = {'width': '90vw', 'height': '90vh', 'margin': '0 auto 0 auto'}
map = gmaps.figure(center=GEI_coordinate, zoom_level=15, layout=figure_layout)

# Add the antenas to the map as markers
markers = gmaps.marker_layer(LoRa_antenas)
map.add_layer(markers)

# Display the data on the map as an heatmap
heatmap_layer = gmaps.heatmap_layer(locations, weights=weights, max_intensity=1, point_radius=10.0)
map.add_layer(heatmap_layer)

# Generate an html file to visualise the data
embed_minimal_html('export.html', views=[map])

print('\nThe html file was generated')
print('\nRun python3 -m http.server 8080 \nThen open http://0.0.0.0:8080/export.html in a browser\n')