from flask import Flask, render_template, request
import socket
import numpy as np 
from sklearn.ensemble import RandomForestClassifier
from scipy.fftpack import fft

frame_rate = 50 # accelerometer sampling rate
duration_s = 2.5 # sample duration in seconds
total_samples = int(frame_rate * duration_s)

gesture_dict = np.load("gesture_dict.npy", allow_pickle=True).item()
clf = None

prediction = ""

# Featurization method
def featurize(arr):
    # calcualte fft for axes
    # need absolute value since fft is complex
    x = np.abs(fft(arr[:, 0]))
    y = np.abs(fft(arr[:, 1]))
    z = np.abs(fft(arr[:, 2]))

    # get top two freq val w highest magnitudes
    # reduces dimensionality, improves robustness
    two_x = np.argsort(x)[-2:]
    two_y = np.argsort(y)[-2:]
    two_z = np.argsort(z)[-2:]

    # combine all into single array as our feature vector
    features = np.concatenate([x[two_x], y[two_y], z[two_z]])
    return features


# Trains classifier
def train_ml_classifier(): 
	clf_model = RandomForestClassifier()
	X = []
	Y = []
	for key in gesture_dict.keys():
		gest_trials = gesture_dict[key]
		for trial in gest_trials:
			X.append(featurize(trial))
			Y.append(key)
	X = np.array(X)
	Y = np.array(Y)
	clf_model.fit(X, Y)
	return clf_model

# Parse the data stream to get the values from it
def parse_data_stream(data_stream):
	data = data_stream.split("\n")
	data_vals = []

	# scan tokens
	for i in range(len(data)):
		imu_vals = data[i]
		
		# if EOD, return
		if "EOD" in imu_vals:
			break
		
		# 3 axes per line
		values = []
		for val in imu_vals.split(" "):
			try:
				values.append(float(val))
			except ValueError:
				break
		if (len(values) == 3):
			data_vals.append(values)
	
	data_vals = np.array(data_vals)
	return data_vals

app = Flask(__name__)

# start TCP server
port = 8080
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)        
print ("Socket successfully created") 

# ENTER IP address
s.bind(('172.26.57.19', port))
print ("socket binded to %s" %(port))

s.listen(5)      
print ("socket is listening")

newSocket, address = s.accept()
print ("Connection from ", address)

# ask particle for new sample
def collectTest():

	# send new command
	cmd = str(total_samples)
	cmd_b = bytearray()
	cmd_b.extend(map(ord, cmd))
	print ("Requesting " + cmd + " samples")
	newSocket.send(cmd_b)

	# receive response
	data_stream = ""
	while 1:
		receivedData = newSocket.recv(1024)
		if not receivedData:
			break
		else:
			data = receivedData.decode('utf-8')
			data_stream = data_stream + data
			
			# Continously receive the data until you hit EOD
			if "EOD\n" in data:
				return data_stream

@app.route("/", methods = ['POST', 'GET'])
def index():
	global prediction, clf, gesture_dict

	# check for post request
	if request.method == "POST":
		print(request.form)

		# if training, receive new data stream and associate with symbol
		if request.form["submit"] == "Train":
			
			# reset classifier
			clf = None
			gesture = request.form["symbol"]
			stream = collectTest()
			if gesture not in gesture_dict.keys():
				gesture_dict[gesture] = []
			gesture_dict[gesture].append(parse_data_stream(stream))
		
		# if testing, receive new data stream and make inference
		elif request.form["submit"] == "Test":
			# Check if there is any data to train! 
			if bool(gesture_dict) == False: 
				print("Need to collect training data first!")
				prediction = "None; need to collect data first!"
			else:
				if clf is None: # If not trained, train a new model
					print("Training new model")
					np.save('gesture_dict.npy',  gesture_dict) 
					clf = train_ml_classifier()
				
				stream = collectTest()
				# Featurization happens here
				X_in = featurize(parse_data_stream(stream)) 
				prediction = clf.predict([X_in])[0]
				print("Prediction:", prediction)
		
		# if reset, delete gesture dict and reset classifier
		elif request.form["submit"] == "Reset":
			print("Deleting all data")
			prediction = ""
			gesture_dict = {}
			clf = None
	
	# count training data
	counts = ""
	print("No predictions yet")
	for key in gesture_dict:
		counts += "(" + key + ", " + str(len(gesture_dict[key])) + ") "
	
	# if prediction present, update page
	if (prediction != ""): 
		return render_template("index.html", pred=prediction, trials=counts)
	
	# else, only output trial count
	return render_template("index.html", trials=counts)

app.run(debug=True, use_reloader=False)