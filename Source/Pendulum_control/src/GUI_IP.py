import time
import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from datetime import datetime

class AnimationPlot:

    def animate(self, i, timeList, dataList1, dataList2, dataList3, ser, file):
        ser.write(b'g')                                     # Transmit the char 'g' to receive the Arduino data point
        arduinoData_string = ser.readline().decode('ascii') # Decode receive Arduino data as a formatted string

        try:
            current_time, arduinoData_float1, arduinoData_float2, arduinoData_float3 = map(float, arduinoData_string.split(','))  # Convert to three floats

            timeList.append(current_time)                   # Add current time to the time list
            dataList1.append(arduinoData_float1)            # Add first number to the first list
            dataList2.append(arduinoData_float2)            # Add second number to the second list
            dataList3.append(arduinoData_float3)            # Add third number to the third list

            file.write(f'{current_time},{arduinoData_float1},{arduinoData_float2},{arduinoData_float3}\n')

        except:                                             # Pass if data point is bad                               
            pass

        timeList = timeList[-50:]                           # Fix the list size so that the animation plot 'window' is x number of points
        dataList1 = dataList1[-50:]
        dataList2 = dataList2[-50:]
        dataList3 = dataList3[-50:]
        x0 = dataList3[-1]
        max_data1 = max(dataList1) if dataList1 else float('-inf')  # Get the maximum value of the last 50 points in dataList1
        min_data1 = min(dataList1) if dataList1 else float('inf')   # Get the minimum value of the last 50 points in dataList1
        max_data2 = max(dataList2) if dataList2 else float('-inf')  # Get the maximum value of the last 50 points in dataList2
        min_data2 = min(dataList2) if dataList2 else float('inf')   # Get the minimum value of the last 50 points in dataList2
        ax.clear()                                          # Clear last data frame

        self.getPlotFormat()
        ax.plot(timeList, dataList1, label='Theta (degree)')        # Plot new data frame for the first list with time
        ax.plot(timeList, dataList2, label='x (mm)')        # Plot new data frame for the second list with time
        ax.plot(timeList, dataList3, label='x_desire')         # Plot new data frame for the third list with time
        ax.legend(loc='upper right')                        # Add a legend to differentiate between the three datasets
        
        ax.text(0.01, 0.95, f'_Theta: {round(-abs(abs(min_data1)-180),3)} -> {round(abs(abs(max_data1)-180),3)}', transform=ax.transAxes, verticalalignment='top')
        ax.text(0.01, 0.85, f'_X: {min_data2} -> {max_data2}', transform=ax.transAxes, verticalalignment='top')
        ax.text(0.01, 0.75, f'_X0: {x0}', transform=ax.transAxes, verticalalignment='top')
        
        # Rotate x-axis labels for better readability
        plt.subplots_adjust(bottom=0.30)                    # Adjust bottom to fit x-axis labels

    def getPlotFormat(self):                           # Set Y axis limit of plot
        ax.set_title("Inverted Pendulum Data")                      # Set title of figure
        ax.set_ylabel("Value")                             # Set title of y axis
        ax.set_xlabel("Time")
        ax.grid(True)                               # Set title of x axis

timeList = []                                              # Create empty list variable for time data
dataList1 = []                                             # Create empty list variable for first dataset
dataList2 = []                                             # Create empty list variable for second dataset
dataList3 = []                                             # Create empty list variable for third dataset

fig = plt.figure()                                         # Create Matplotlib plots fig is the 'higher level' plot window
ax = fig.add_subplot(111)                                  # Add subplot to main fig window

realTimePlot = AnimationPlot()

ser = serial.Serial("COM9", 9600)                          # Establish Serial object with COM port and BAUD rate to match Arduino Port/rate
time.sleep(2)

with open('data_log.txt', 'w') as file:
    # Write the header line
    file.write('Time,Data1,Data2,Data3\n')
    
                                                 # Time delay for Arduino Serial initialization 

                                                           # Matplotlib Animation Function that takes care of real-time plot.
                                                           # Note that 'fargs' parameter is where we pass in our dataLists and Serial object. 
    ani = animation.FuncAnimation(fig, realTimePlot.animate, frames=100, fargs=(timeList, dataList1, dataList2, dataList3, ser, file), interval=100) 

    plt.show()                                                 # Keep Matplotlib plot persistent on screen until it is closed
    ser.close()
