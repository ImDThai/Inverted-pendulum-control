import pandas as pd

def analyze_pendulum_data(file_path):
    # Load the data from the file
    data = pd.read_csv(file_path, delimiter=",", header=0, names=["Time", "Angle (degrees)", "Position (mm)"])

    # Extract the last 10 rows of the dataset
    last_10_rows = data.tail(30)

    # Find the maximum and minimum values for Angle and Position in the last 10 rows
    max_angle = last_10_rows["Angle (degrees)"].max()
    min_angle = last_10_rows["Angle (degrees)"].min()
    max_position = last_10_rows["Position (mm)"].max()
    min_position = last_10_rows["Position (mm)"].min()

    return max_angle, min_angle, max_position, min_position


with open('Analyza_Data.txt', 'w') as file:
# Write the header line
    for i in range(2, 47):
        file_path = f'data_log{i}.txt'
        max_angle, min_angle, max_position, min_position = analyze_pendulum_data(file_path)
        file.write(f'{min_position},{max_position},{min_angle},{max_angle}\n')
