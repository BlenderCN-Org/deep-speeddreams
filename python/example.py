import cv2
import speed_dreams as sd

# Create shared memory object
Memory = sd.CSharedMemory()

# Enable Pause-Mode
Memory.setSyncMode(True)

IsExit = False

# Repeat until ESC was pressed
while not IsExit:

  # Check if new values are available
  if Memory.read():
    print("\n")
    print("Image Size: {}x{}".format(Memory.Data.Image.ImageWidth, Memory.Data.Image.ImageHeight))

    # Output all values from memory
    print(str(Memory.Data))

    # Output image from memory
    cv2.imshow("SpeedDreams Image", Memory.Image)

    # Indicate that reader process is ready and a new value can be written
    Memory.indicateReady()

  if cv2.waitKey(10) == 27:
    IsExit = True