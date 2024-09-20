#pragma once
#include "python_toolbox.h"
#include "python_numpy_wrappers.h"

#include "hoNDArray.h"
#include "mri_core_data.h"
#include "log.h"

#include <boost/python.hpp>
namespace bp = boost::python;

namespace Gadgetron {

class IsmrmrdReconData_to_python_object {
public:
  static PyObject* convert(const mrd::ReconData & reconData) {
//      initialize_python();
      GILLock lock;
    bp::object pygadgetron = bp::import("gadgetron");

    auto pyReconData = bp::list();
    for (auto & reconBit : reconData.rbits ){
      auto data = DataBufferedToPython(reconBit.data);
      auto ref = 	reconBit.ref ? DataBufferedToPython(*reconBit.ref) : bp::object();

      auto pyReconBit = pygadgetron.attr("IsmrmrdReconBit")(data,ref);
      pyReconData.append(pyReconBit);

    }
    // increment the reference count so it exists after `return`
    return bp::incref(pyReconData.ptr());
  }

private:
  static bp::object DataBufferedToPython( const mrd::BufferedData & dataBuffer){
    bp::object pygadgetron = bp::import("gadgetron");
    auto data = bp::object(dataBuffer.data);
    auto headers = boost::python::object(dataBuffer.headers);
    auto trajectory = bp::object(dataBuffer.trajectory);
    auto sampling = SamplingDescriptionToPython(dataBuffer.sampling);
    auto buffer = pygadgetron.attr("BufferedData")(data,headers,sampling,trajectory);

    bp::incref(data.ptr());
    bp::incref(headers.ptr());
    bp::incref(trajectory.ptr());
    bp::incref(sampling.ptr());

    return buffer;
  }

  static bp::object SamplingDescriptionToPython(const mrd::SamplingDescription & sD){
    bp::object pygadgetron = bp::import("gadgetron");
    bp::object result;
    try {
     auto tmp = pygadgetron.attr("SamplingDescription");
     result = tmp();
     result.attr("encoded_FOV") = bp::make_tuple(sD.encoded_fov.x, sD.encoded_fov.y, sD.encoded_fov.z);
     result.attr("encoded_matrix") = bp::make_tuple(sD.encoded_matrix.x, sD.encoded_matrix.y, sD.encoded_matrix.z);
     result.attr("recon_FOV") = bp::make_tuple(sD.recon_fov.x, sD.recon_fov.y, sD.recon_fov.z);
     result.attr("recon_matrix") = bp::make_tuple(sD.recon_matrix.x, sD.recon_matrix.y, sD.recon_matrix.z);
 } catch (bp::error_already_set const &){
        std::string err = pyerr_to_string();
        GERROR(err.c_str());
        throw std::runtime_error(err);
    }
     std::vector<bp::object> SL;

        auto sampling_limit_ro = pygadgetron.attr("SamplingLimit")();
        sampling_limit_ro.attr("min") = sD.sampling_limits.ro.minimum;
        sampling_limit_ro.attr("max") = sD.sampling_limits.ro.maximum;
        sampling_limit_ro.attr("center") = sD.sampling_limits.ro.center;
        SL.push_back(sampling_limit_ro);

        auto sampling_limit_e1 = pygadgetron.attr("SamplingLimit")();
        sampling_limit_e1.attr("min") = sD.sampling_limits.e1.minimum;
        sampling_limit_e1.attr("max") = sD.sampling_limits.e1.maximum;
        sampling_limit_e1.attr("center") = sD.sampling_limits.e1.center;
        SL.push_back(sampling_limit_e1);

        auto sampling_limit_e2 = pygadgetron.attr("SamplingLimit")();
        sampling_limit_e2.attr("min") = sD.sampling_limits.e2.minimum;
        sampling_limit_e2.attr("max") = sD.sampling_limits.e2.maximum;
        sampling_limit_e2.attr("center") = sD.sampling_limits.e2.center;
        SL.push_back(sampling_limit_e2);

      result.attr("sampling_limits") = bp::make_tuple(SL[0], SL[1], SL[2]);

      return result;
  }
};


/// Used for making an hoNDArray from a NumPy array
struct IsmrmrdReconData_from_python_object {
  IsmrmrdReconData_from_python_object() {
    // actually register this converter with Boost
    bp::converter::registry::push_back(
        &convertible,
        &construct,
        bp::type_id<mrd::ReconData>());
  }

  /// Returns NULL if the object is not convertible. Or well.... it should
  static void* convertible(PyObject* obj) {
    return obj;
  }

  /// Construct an hoNDArray in-place
  static void construct(PyObject* obj, bp::converter::rvalue_from_python_stage1_data* data) {
    void* storage = ((bp::converter::rvalue_from_python_storage<mrd::ReconData >*)data)->storage.bytes;
    mrd::ReconData* reconData = new (storage) mrd::ReconData;
    data->convertible = storage;


    try {
      bp::list pyRecondata((bp::handle<>(bp::borrowed(obj))));
      auto length = bp::len(pyRecondata);
      GDEBUG("Recon data length: %i\n",length);
      for (int i = 0; i < length; i++){
        bp::object reconBit = pyRecondata[i];
        mrd::ReconBit rBit;
        rBit.data = extractDataBuffered(reconBit.attr("data"));
        if (PyObject_HasAttrString(reconBit.ptr(),"ref")){
          rBit.ref = extractDataBuffered(reconBit.attr("ref"));
        }
        reconData->rbits.push_back(rBit);
      }

    }catch (const bp::error_already_set&) {
      std::string err = pyerr_to_string();
      GERROR(err.c_str());
      throw std::runtime_error(err);
    }
  }
  static mrd::BufferedData extractDataBuffered(bp::object pyDataBuffered){
    mrd::BufferedData result;

    result.data = bp::extract<hoNDArray<std::complex<float>>>(pyDataBuffered.attr("data"))();
    if (PyObject_HasAttrString(pyDataBuffered.ptr(),"trajectory"))
      result.trajectory = bp::extract<hoNDArray<float>>(pyDataBuffered.attr("trajectory"))();

    result.headers = bp::extract<hoNDArray<mrd::AcquisitionHeader>>(pyDataBuffered.attr("headers"))();

    auto pySampling = pyDataBuffered.attr("sampling");
    mrd::SamplingDescription sampling;
    sampling.encoded_fov.x = bp::extract<float>(pySampling.attr("encoded_FOV")[0]);
    sampling.encoded_fov.y = bp::extract<float>(pySampling.attr("encoded_FOV")[1]);
    sampling.encoded_fov.z = bp::extract<float>(pySampling.attr("encoded_FOV")[2]);
    sampling.encoded_matrix.x = bp::extract<uint32_t>(pySampling.attr("encoded_matrix")[0]);
    sampling.encoded_matrix.y = bp::extract<uint32_t>(pySampling.attr("encoded_matrix")[1]);
    sampling.encoded_matrix.z = bp::extract<uint32_t>(pySampling.attr("encoded_matrix")[2]);
    sampling.recon_fov.x = bp::extract<float>(pySampling.attr("recon_FOV")[0]);
    sampling.recon_fov.y = bp::extract<float>(pySampling.attr("recon_FOV")[1]);
    sampling.recon_fov.z = bp::extract<float>(pySampling.attr("recon_FOV")[2]);
    sampling.recon_matrix.x = bp::extract<uint32_t>(pySampling.attr("recon_matrix")[0]);
    sampling.recon_matrix.y = bp::extract<uint32_t>(pySampling.attr("recon_matrix")[1]);
    sampling.recon_matrix.z = bp::extract<uint32_t>(pySampling.attr("recon_matrix")[2]);

    auto pySLro = pySampling.attr("sampling_limits")[0];
    sampling.sampling_limits.ro.minimum = bp::extract<uint32_t>(pySLro.attr("min"));
    sampling.sampling_limits.ro.center = bp::extract<uint32_t>(pySLro.attr("center"));
    sampling.sampling_limits.ro.maximum = bp::extract<uint32_t>(pySLro.attr("max"));
    auto pySLe1 = pySampling.attr("sampling_limits")[0];
    sampling.sampling_limits.e1.minimum = bp::extract<uint32_t>(pySLe1.attr("min"));
    sampling.sampling_limits.e1.center = bp::extract<uint32_t>(pySLe1.attr("center"));
    sampling.sampling_limits.e1.maximum = bp::extract<uint32_t>(pySLe1.attr("max"));
    auto pySLe2 = pySampling.attr("sampling_limits")[0];
    sampling.sampling_limits.e2.minimum = bp::extract<uint32_t>(pySLe2.attr("min"));
    sampling.sampling_limits.e2.center = bp::extract<uint32_t>(pySLe2.attr("center"));
    sampling.sampling_limits.e2.maximum = bp::extract<uint32_t>(pySLe2.attr("max"));

    result.sampling = sampling;
    return result;
  }


};


/// Partial specialization of `python_converter` for hoNDArray
template<> struct python_converter<mrd::ReconData> {
  static void create()
  {
    // register hoNDArray converter
    bp::type_info info = bp::type_id<mrd::ReconData >();
    const bp::converter::registration* reg = bp::converter::registry::query(info);
    // only register if not already registered!
    if (nullptr == reg || nullptr == (*reg).m_to_python) {
      bp::to_python_converter<mrd::ReconData, IsmrmrdReconData_to_python_object >();
      IsmrmrdReconData_from_python_object();
    }

  }
};

}
