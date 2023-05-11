# Particle Gesture Detection and Classification - README

## Table of Contents
1. [Overview](#overview)
   * [Description](#description)
   * [Components](#components)
2. [Getting Started](#getting-started)
   * [Hardware Requirements](#hardware-requirements)
   * [Prerequisites](#prerequisites)
   * [Installing Required Libraries](#installing-required-libraries)
   * [Setting up Classification Harness Code](#setting-up-classification-harness-code)
   * [Running the Gesture Detection Code](#running-the-gesture-detection-code)
   * [Running the Gesture Classification Code](#running-the-gesture-classification-code)
3. [Repository Structure](#repository-structure)
   * [Directory](#directory)
   * [License](#license)


## Overview
### Description
The Particle Gesture Detection and Classification system leverages the Particle Argon microcontroller and the Qwiic accelerometer to detect various hand gestures. By analyzing the accelerometer data and applying preprocessing techniques, the system can accurately recognize gestures such as lines, circles, squares, triangles, and alphabet characters drawn in the air. The system offers the capability to send the detected gestures to the Particle cloud for further processing. 

This system additionally adopts a comprehensive approach that includes collecting accelerometer data, extracting meaningful features, training machine learning models, and performing live inference to classify gestures in real-time. By leveraging these techniques, the system has a pipeline that has an average of 97%+ accuracy in a Leave-One-Trial Out Cross Validation across gestures across 3 different classifiers (eg. SVC, KNeighborsClassifier, RandomForestClassifier).

### Components
The project consists of two main components:
* **Gesture Detection**: The Particle Argon and Qwiic accelerometer combination enables gesture detection by sampling raw data and applying basic filtering techniques directly on the microcontroller. Detected gestures are associated with specific colors on a Grove chainable LED, providing visual notifications. The system connects to the Particle cloud, allowing the transmission of detected gestures using Particle PubSub functionality. Gesture data can be easily subscribed and retrieved using the "particle subscribe" command in a terminal. Particle Functions are utilized to selectively send specific gestures over the pubsub channel for efficient and targeted gesture communication.
* **Gesture Classification**: The system includes a machine learning component implemented in Python. The collected gesture data is sent to a Python server where it undergoes featurization and is used to train various machine learning classifiers. The trained models are then used for live inference, allowing real-time recognition of gestures based on the featurized data.

## Getting Started
To get started with the Particle Gesture Detection and Classification system, follow the steps below:

### Hardware Requirements
* Particle Argon microcontroller
* Particle-Grove Shield
* Sparkfun Qwiic Shield
* Qwiic Accelerometer (MMA8452Q based)
* 1 x Qwiic Cable
* 1 x RGB LED

### Prerequisites
Ensure that you have the following installed on your system:
* Python 3.6 or higher
* NumPy
* SciPy
* scikit-learn

Ensure that you have the following account:
* Particle Account

Additionally, you will need to set up your Particle device and claim it to your Particle account. Follow the instructions provided by Particle to complete the setup process [here](https://docs.particle.io/device-setup/).

### Installing Required Libraries
To install the required library, open a terminal and run the following command:
```bash 
pip install numpy scipy scikit-learn matplotlib 
```

### Setting up Classification Harness Code 
To install all the dependencies in order to use the gesture classifier, make sure you are in the right folder and run:
```bash 
python3 -m pip install -r requirements.txt 
```

### Running the Gesture Detection Code
1. After assembling the hardware components, proceed to the Particle Web IDE to create a new project by clicking the "Create New Project" button.
2. Provide a name for your project, and then copy and paste the code from the Gesture Detector Code folder's `gesture_detector.ino` file into the Particle Web IDE editor.
3. Ensure that your Particle Argon is selected as the target device for flashing.
4. Click the "Flash" button to upload the firmware code to your Particle Argon.
5. Once the code is successfully uploaded, you can start making gestures using the accelerometer.
    * To view the detected gestures, you have two options:
    * Check the events on the Particle console.
    * Use the serial monitor by typing the following command in your terminal: 
    ```bash 
    particle serial monitor 
    ```
**Note**: Ensure that your Particle Argon is properly connected and powered on while running the code.

### Running the Gesture Classification Code
1. Before running the TCP server, retrieve the IP address of the device where you will run the Python files. Open a terminal and enter the following command to obtain your IP address:
```bash 
ipconfig getifaddr en0
```
2. Update the IP address in both `particle_server.py` and the `tcp_accel_sampler.ino` files from the Gesture Classifier Code folder.
3. Start the Flask/TCP backend by running the command 
```bash 
python3 particle_server.py
```
4. Flash the Particle code to your Particle device by copying the code from the file and updating the server IP. Note that the Python server must be running and displaying the message "socket is listening" before the Particle device attempts to connect to it.
5. Once the Particle device is connected to the TCP backend, Flask will launch a web app accessible at `http://localhost:5000`. Confirmation of the web app's launch will be displayed in the terminal.
6. On the web app, you will find a dropdown menu to select a gesture for training. Choose a gesture and click "Train" to initiate the training process.
7. After clicking "Train", the on-board LED of the Particle device will flash twice, indicating the start of gesture recording on the accelerometer for a duration of 2.5 seconds.
8. Once sufficient data has been collected, you can test and make inferences by clicking "Test". The on-board LED will flash twice again, indicating the recording of a gesture. The web app will update with the latest prediction.
9. Clicking "Reset" will clear all recorded test data.
10. To assess the accuracy of the featurization code against the three classifiers, you can run the Jupyter file `visualize.ipynb` or`visualize.py`.

**Note**: Ensure that the Particle device is connected and the Python server is running before performing the above steps.

## Repository Structure

### Directory


### License
[MIT](https://choosealicense.com/licenses/mit/)
