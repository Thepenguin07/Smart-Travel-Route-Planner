# Smart Travel Route Planner (GTK+ 3)

A desktop application written in C that utilizes graph theory to calculate the most efficient travel routes between cities. Whether you want the shortest drive or the fastest trip, this tool provides the data using industry-standard algorithms.



## 🚀 Features

- **Dual Optimization Modes:** Calculate paths based on distance (miles) or time (minutes).
- **Advanced Graph Algorithms:** - **Dijkstra's Algorithm:** Finds the single-source shortest path for specific city pairs.
    - **Floyd-Warshall Algorithm:** Generates a complete matrix showing the shortest routes between every city in the dataset.
- **Dynamic Map Loading:** Load custom map files at runtime via a file chooser.
- **Data Visualization:** View a list of all loaded cities and direct routes available in the graph.
- **User Authentication:** Secure login screen (Supports multi-user roles).

## 🛠️ Tech Stack

- **Language:** C
- **UI Framework:** GTK+ 3.0
- **Layout:** GtkGrid, GtkBox, GtkHeaderBar
- **Data Structure:** Adjacency Matrix (Graph)

## 📋 Prerequisites

To compile and run this application, you need the GTK+ 3 development libraries installed on your system.

**For Ubuntu/Debian:**
bash
sudo apt-get install libgtk-3-dev

📖 How to Use
Login:

Admin: Username: admin | Password: pass123

User: Username: user | Password: 1234

Load a Map: Click the "Load Map File..." button in the header bar.

Map File Format: The application expects a text file formatted as follows:

Line 1: Number of cities (N).

Next N lines: Names of the cities.

Next line: Number of routes (M).

Next M lines: City1,City2,Distance,Time

Find Routes: Enter the source and destination city names, select your optimization preference, and click "Find Shortest Path."

📂 Project Structure
struct Graph: Stores city names and an adjacency matrix of Edge structs.

dijkstra(): Core logic for point-to-point pathfinding.

getFloydWarshallString(): Logic for calculating the all-pairs matrix.

create_main_application_window(): Handles the GTK UI construction.

🤝 Contributing
Feel free to fork this repository, open issues, or submit pull requests to improve the pathfinding logic or UI design!

