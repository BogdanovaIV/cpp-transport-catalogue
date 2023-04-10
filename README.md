# cpp-transport-catalogue
Final project: Transportation guidebook

This is a transportation guidebook that takes in a JSON file containing information about public transportation routes and stops. Each stop contains information about the name, latitude, longitude, and distance to the nearest stop in meters. Each route contains information about whether it is circular, as well as a list of stops, including the name of each stop, its coordinates, and the distance to the next stop.

After processing the request, the system returns a response in JSON format containing information about stops, routes, and a route map in SVG format. The route map visualizes all stops on the route and the connections between them.

Using this system, you can quickly and conveniently obtain information about public transportation routes and stops, which simplifies orientation in the city and trip planning.

Sure, here is an example input file in JSON format:
{
    "base_requests": [
        {
            "type": "Bus",
            "name": "114",
            "stops": ["Морской вокзал", "Ривьерский мост"],
            "is_roundtrip": false
        },
        {
            "type": "Stop",
            "name": "Ривьерский мост",
            "latitude": 43.587795,
            "longitude": 39.716901,
            "road_distances": {"Морской вокзал": 850}
        },
        {
            "type": "Stop",
            "name": "Морской вокзал",
            "latitude": 43.581969,
            "longitude": 39.719848,
            "road_distances": {"Ривьерский мост": 850}
        }
    ],
    "render_settings": {
        "width": 200,
        "height": 200,
        "padding": 30,
        "stop_radius": 5,
        "line_width": 14,
        "bus_label_font_size": 20,
        "bus_label_offset": [7, 15],
        "stop_label_font_size": 20,
        "stop_label_offset": [7, -3],
        "underlayer_color": [255,255,255,0.85],
        "underlayer_width": 3,
        "color_palette": ["green", [255,160,0],"red"]
    },
    "stat_requests": [
        { "id": 1, "type": "Map" },
        { "id": 2, "type": "Stop", "name": "Ривьерский мост" },
        { "id": 3, "type": "Bus", "name": "114" }
    ]
}

Output file in JSON:

[
    {
        "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n  <polyline points=\"100.817,170 30,30 100.817,170\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <circle cx=\"100.817\" cy=\"170\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"30\" cy=\"30\" r=\"5\" fill=\"white\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"black\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ривьерский мост</text>\n  <text fill=\"black\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ривьерский мост</text>\n</svg>",
        "request_id": 1
    },
    {
        "buses": [
            "114"
        ],
        "request_id": 2
    },
    {
        "curvature": 1.23199,
        "request_id": 3,
        "route_length": 1700,
        "stop_count": 3,
        "unique_stop_count": 2
    }
]
