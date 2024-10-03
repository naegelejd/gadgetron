#pragma once

#include "Gadget.h"
#include "hoNDArray.h"

#include <ismrmrd/ismrmrd.h>
#include <complex>

namespace Gadgetron{

  template <typename T> class DeviceChannelSplitterGadget : 
  public Gadget2<ISMRMRD::ImageHeader,hoNDArray< T > >
  {
  protected:
    virtual int process(GadgetContainerMessage<ISMRMRD::ImageHeader>* m1, 
			GadgetContainerMessage< hoNDArray< T > >* m2);
  };
  
  class DeviceChannelSplitterGadgetUSHORT :
  public DeviceChannelSplitterGadget<uint16_t>
  {
  public:
    GADGET_DECLARE(DeviceChannelSplitterGadgetUSHORT);
  };

  class DeviceChannelSplitterGadgetFLOAT :
  public DeviceChannelSplitterGadget<float>
  {
  public:
    GADGET_DECLARE(DeviceChannelSplitterGadgetFLOAT);
  };

  class DeviceChannelSplitterGadgetCPLX :
  public DeviceChannelSplitterGadget< std::complex<float> >
  {
  public:
    GADGET_DECLARE(DeviceChannelSplitterGadgetCPLX);
  };
}

