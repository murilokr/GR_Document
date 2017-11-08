# GR_Document (Gesture Recognition on Documents)
This software was made for my undergraduate thesis, it uses Neural Networks and Hidden Markov Models to recognize gestures and apply the gestures actions to pdf. documents that have window focus.

Available Gestures:

    Advance   (Right Hand closed swipe to the right)
    Return    (Left Hand closed swipe to the left)
    Zoom-In   (Both Hands opened swipe downwards)
    Zoom-Out  (Both Hands with a point gesture swipe upwards)

# Installation Guide

Require **Ubuntu 16.04 LTS** or higher

Dependencies | Download
-------------|----------
OpenCV		 | https://github.com/opencv/opencv
OpenNI       | https://github.com/OpenNI/OpenNI
PrimeSensor	 | https://github.com/avin2/SensorKinect
cmake 3.5.1	 | `sudo apt-get install cmake`
FANN		 | https://github.com/libfann/fann
CvHMM		 | Included in this repository


# Installing OpenNI and PrimeSensor
1. Install OpenNI dependencies as stated in their github repository.
2. Clone OpenNI repository:
	```
    cd ~/openni
    git clone https://github.com/OpenNI/OpenNI.git
    ```
3.	Run RedistMaker:
	```
    cd ./Platform/Linux/CreateRedist/
    ./RedistMaker
    ```
4.	Compile OpenNI:
	```
    cd ../Redist/
    sudo ./install.sh
    ```
5.	Clone PrimeSensor repository:
	```
    cd ~/primesensor
    git clone https://github.com/avin2/SensorKinect.git
    ```
6.	Build:
	```
    cd ./Platform/Linux/CreateRedist
    ./RedistMaker
    cd ../Redist/
    sudo ./install.sh
    ```


# Installing OpenCV and Integration with OpenNI
*Note: if having trouble following this, try to follow the [official guide](https://docs.opencv.org/2.4/doc/user_guide/ug_kinect.html)*
1. Clone OpenCV repository:
	```
    cd ~/opencv
    git clone https://github.com/opencv/opencv.git 
    ```
2. Build the Source:
	```
    cd ~/opencv
    mkdir release
    cd release/
    cmake-gui
    ```
3. Set the source directory and click configure.
4. Set **WITH_OPENNI** flag.
5. Build.

# Installing FANN
1. Clone the repository:
	```
    cd ~/fann
    git clone https://github.com/libfann/fann.git
    ```
2. Build:
	```
    cmake .
    sudo make install
    ```
    

