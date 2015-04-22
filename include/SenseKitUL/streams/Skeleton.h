#ifndef SKELETON_H
#define SKELETON_H

#include <SenseKit/SenseKit.h>
#include <stdexcept>
#include <SenseKitUL/skul_ctypes.h>
#include <SenseKitUL/streams/skeleton_capi.h>
#include <SenseKitUL/Vector.h>

namespace sensekit {

    class Skeleton
    {
    public:
        explicit Skeleton(sensekit_skeleton_t skeleton) :
            m_skeleton(skeleton)
        { }

        inline int32_t trackingId() const { return m_skeleton.trackingId; }
        inline sensekit_skeleton_status status() const { return m_skeleton.status; }

    private:
        sensekit_skeleton_t m_skeleton;
    };

    using SkeletonList = std::vector<Skeleton>;

    class SkeletonStream : public DataStream
    {
    public:
        explicit SkeletonStream(sensekit_streamconnection_t connection)
            : DataStream(connection)
        { }

        static const sensekit_stream_type_t id = SENSEKIT_STREAM_SKELETON;
    };

    class SkeletonFrame
    {
    public:
        SkeletonFrame(sensekit_reader_frame_t readerFrame, sensekit_stream_subtype_t subtype)
        {
            if (readerFrame != nullptr)
            {
                sensekit_frame_get_skeletonframe(readerFrame, &m_skeletonFrame);
                sensekit_skeletonframe_get_frameindex(m_skeletonFrame, &m_frameIndex);

                size_t maxSkeletonCount;
                sensekit_skeletonframe_get_skeleton_count(m_skeletonFrame, &maxSkeletonCount);

                m_skeletons.reserve(maxSkeletonCount);
            }
        }

        bool is_valid() { return m_skeletonFrame != nullptr; }

        size_t skeleton_count()
        {
            throwIfInvalidFrame();
            verify_skeletonlist();
            return m_skeletons.size();
        }

        const SkeletonList& skeleton()
        {
            throwIfInvalidFrame();
            verify_skeletonlist();
            return m_skeletons;
        }

        sensekit_frame_index_t frameIndex() { throwIfInvalidFrame(); return m_frameIndex; }

    private:
        void throwIfInvalidFrame()
        {
            if (m_skeletonFrame == nullptr)
            {
                throw std::logic_error("Cannot operate on an invalid frame");
            }
        }

        void verify_skeletonlist()
        {
            if (m_skeletonsInitialized)
            {
                return;
            }

            m_skeletonsInitialized = true;

            sensekit_skeleton_t* skeletonPtr;
            size_t skeletonCount;

            sensekit_skeletonframe_get_skeletons_ptr(m_skeletonFrame, &skeletonPtr, &skeletonCount);

            for (int i = 0; i < skeletonCount; ++i, ++skeletonPtr)
            {
                sensekit_skeleton_t& p = *skeletonPtr;
                if (p.status != sensekit_skeleton_status::SENSEKIT_SKELETON_STATUS_NOT_TRACKED)
                {
                    m_skeletons.push_back(Skeleton(p));
                }
            }
        }

        bool m_skeletonsInitialized{false};
        SkeletonList m_skeletons;
        sensekit_skeletonframe_t m_skeletonFrame{nullptr};
        sensekit_frame_index_t m_frameIndex;
    };
}

#endif /* SKELETON_H */
