import mmap
import ctypes
import numpy as np
import cv2

from .data_types import Data_t, RECORD_MEMORY_NAME, RECORD_IMAGE_CHANNELS

class CSharedMemory():
  TARGET_IMAGE_CHANNELS = RECORD_IMAGE_CHANNELS


  def __init__(self, TargetResolution = [640, 480]):
    """ 
    Creates a Shared-Memory class.
     
    Keyword arguments:
       TargetResolution: The needed resolution of the image (default 640x480 px)
    """
    self._IsPauseOn = False
    self._TargetResolution = TargetResolution
    self._SharedMemory = mmap.mmap(-1, ctypes.sizeof(Data_t), RECORD_MEMORY_NAME)
    self._Data = Data_t.from_buffer(self._SharedMemory)
    self._Image = np.empty(shape=TargetResolution + [self.TARGET_IMAGE_CHANNELS])
    self._RawImage = np.empty(shape=TargetResolution + [self.TARGET_IMAGE_CHANNELS])


  def __del__(self):
    self.Data.Sync.IsPauseOn = 0
    self.Data.Control.IsControlling = 0


  def setSyncMode(self, IsPauseOn = True):
    """
    Enables or disables the pause mode (Speed-Dreams waits with 
    new data until old data have been read).
    
    Keyword arguments:
      IsPauseOn: Indicates, if pause mode should be enabled or disabled.
    """
    self._IsPauseOn = IsPauseOn


  def read(self):
    """
    Checks if new data is available and read an image from the memory if possible.
    
    :return: Returns True if data can be read.
    """
    if self.Data.Sync.IsWritten == 1:

      if self._IsPauseOn:
        self.Data.Sync.IsPauseOn = 1
      else:
        self.Data.Sync.IsPauseOn = 0

      Width = self.Data.Image.ImageWidth
      Height = self.Data.Image.ImageHeight
      Image = np.fromstring(self.Data.Image.Data, np.uint8, Width * Height * self.TARGET_IMAGE_CHANNELS)
      Image = Image.reshape(Height, Width, self.TARGET_IMAGE_CHANNELS)

      if Width != self._TargetResolution[0] or Height != self._TargetResolution[1]:
        Image = cv2.resize(Image, (self._TargetResolution[0], self._TargetResolution[1]))

      self._RawImage = Image
      self._Image = cv2.flip(Image, 0)
      return True

    return False


  def indicateReady(self):
    """
    Indicates to Speed-Dreams, that the reader is ready.
    """
    self.Data.Sync.IsWritten = 0


  @property
  def Data(self):
    return self._Data


  @property
  def Image(self):
    return self._Image


  @property
  def RawImage(self):
    return self._RawImage


  @property
  def ImageWidth(self):
    return self._TargetResolution[0]

  @property
  def ImageHeight(self):
    return self._TargetResolution[1]