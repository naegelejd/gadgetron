#ifndef MRI_CORE_ACQUISITION_BUCKET_H
#define MRI_CORE_ACQUISITION_BUCKET_H

/** TODO Joe: Remove this file entirely. It is replaced by MRDv2 and mri_core_data.h */

#include "GadgetContainerMessage.h"
#include "mri_core_data.h"

namespace Gadgetron {

  /** 
      This class functions as a storage unit for statistics related to
      the @IsmrmrdAcquisitionData objects.

   */
  struct DeprecatedAcquisitionBucketStats {

      // Set of labels found in the data or ref part of a bucket
      //11D, fixed order [RO, E1, E2, CHA, SLC, PHS, CON, REP, SET, SEG, AVE]
      std::set<uint32_t> kspace_encode_step_1;
      std::set<uint32_t> kspace_encode_step_2;
      std::set<uint32_t> slice;
      std::set<uint32_t> phase;
      std::set<uint32_t> contrast;
      std::set<uint32_t> repetition;
      std::set<uint32_t> set;
      std::set<uint32_t> segment;
      std::set<uint32_t> average;
      void add_stats( const mrd::AcquisitionHeader& header) {
          average.insert(header.idx.average.value_or(0));
          kspace_encode_step_1.insert(header.idx.kspace_encode_step_1.value_or(0));
          kspace_encode_step_2.insert(header.idx.kspace_encode_step_2.value_or(0));
          slice.insert(header.idx.slice.value_or(0));
          contrast.insert(header.idx.contrast.value_or(0));
          phase.insert(header.idx.phase.value_or(0));
          repetition.insert(header.idx.repetition.value_or(0));
          set.insert(header.idx.set.value_or(0));
          segment.insert(header.idx.segment.value_or(0));
      }

  };


  /**

     This class serves as the storage unit for buffered data. 
     The @IsmrmrdAcquisitionData structure contains pointers 
     to the GadgetContainerMessages with the data. 

     Data stored in these buckets will automatically get deleted when the object is
     destroyed. 

   */ 
  struct DeprecatedAcquisitionBucket {
    std::vector< Core::Acquisition > data_;
    std::vector< Core::Acquisition > ref_;
    std::vector<DeprecatedAcquisitionBucketStats> datastats_;
    std::vector<DeprecatedAcquisitionBucketStats> refstats_;
    std::vector< Core::Waveform > waveform_;


      void add_acquisition(Core::Acquisition acq) {
          auto espace = size_t{acq.head.encoding_space_ref.value_or(0)};

        if (acq.head.flags.HasFlags(mrd::AcquisitionFlags::kIsParallelCalibration) || acq.head.flags.HasFlags(mrd::AcquisitionFlags::kIsParallelCalibrationAndImaging)) {
              ref_.push_back(acq);
              if (refstats_.size() < (espace + 1)) {
                  refstats_.resize(espace + 1);
              }
              refstats_[espace].add_stats(acq.head);
          }
        if (!(acq.head.flags.HasFlags(mrd::AcquisitionFlags::kIsParallelCalibration) || acq.head.flags.HasFlags(mrd::AcquisitionFlags::kIsPhasecorrData))) {
              if (datastats_.size() < (espace + 1)) {
                  datastats_.resize(espace + 1);
              }
              datastats_[espace].add_stats(acq.head);
              data_.emplace_back(std::move(acq));
          }
      }

  };

  } // namespace Gadgetron
#endif //MRI_CORE_ACQUISITION_BUCKET_H
