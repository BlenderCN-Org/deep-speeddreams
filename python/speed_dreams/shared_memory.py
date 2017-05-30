import mmap
import ctypes
import numpy as np
import cv2
import math

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

      Width       = self.Data.Image.ImageWidth
      Height      = self.Data.Image.ImageHeight

      Image = np.fromstring(self.Data.Image.Data, np.uint8, Width * Height * self.TARGET_IMAGE_CHANNELS)
      Image = Image.reshape(Height, Width, self.TARGET_IMAGE_CHANNELS)

      AspectRatio = Width / Height
      TargetWidth = int(self._TargetResolution[1] * AspectRatio)

      if TargetWidth >= self._TargetResolution[0]:
        if Width != TargetWidth or Height != self._TargetResolution[1]:
          Image = cv2.resize(Image, (TargetWidth, self._TargetResolution[1]))

        if TargetWidth != self._TargetResolution[0]:
          XStart = int(TargetWidth/2 - self._TargetResolution[0]/2)
          XStop  = int(TargetWidth/2 + self._TargetResolution[0]/2)
          Image = Image[:, XStart:XStop]

      else:
        TargetHeight = int(self._TargetResolution[0]/AspectRatio)

        if Width != self._TargetResolution[0] or Height != TargetHeight:
          Image = cv2.resize(Image, (self._TargetResolution[1], TargetHeight))

          if TargetHeight != self._TargetResolution[1]:
            YStart = int(TargetHeight/2 - self._TargetResolution[1]/2)
            YStop  = int(TargetHeight/2 + self._TargetResolution[1]/2)
            Image = Image[YStart:YStop, :]

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