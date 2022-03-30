from decouple import config
from ipywidgets.embed import embed_minimal_html
import pandas as pd
import gmaps

# Put the value of your Google API key inside a .env file 
gmaps.configure(api_key=config('GOOGLE_API_KEY'))

def min_max_scaling(series):
    return (series - series.min()) / (series.max() - series.min())

data = pd.read_csv(r'data.csv')
data['rssi'] = min_max_scaling(data['rssi'])

locations = data[['lattitude', 'longitude']]
weights = data['rssi']

# Coordonates of key points
GEI_coordinate = (43.570819, 1.466246) 
LoRa_antenas = [(43.570819, 1.466246)]

# Create the google maps figure
figure_layout = {'width': '700px', 'height': '700px', 'margin': '0 auto 0 auto'}
map = gmaps.figure(center=GEI_coordinate, zoom_level=15, layout=figure_layout)

# Add the antenas to the map as markers
markers = gmaps.marker_layer(LoRa_antenas)
map.add_layer(markers)

# Display the data on the map as an heatmap
heatmap_layer = gmaps.heatmap_layer(locations, weights=weights, max_intensity=1, point_radius=7.0)
map.add_layer(heatmap_layer)

# Generate an html file to visualise the data
embed_minimal_html('export.html', views=[map])

print('\nThe html file was generated')
print('\nRun python3 -m http.server 8080 \nThen open http://0.0.0.0:8080/export.html in a browser\n')