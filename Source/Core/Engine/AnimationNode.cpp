// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

    const uint cInvalidFrameId = uint(-1);

    void PrintTabs(uint tabs)
    {
        for (uint i = 0; i < tabs; ++i)
            DebugPrint("|   ");
    }

    void LerpFrame(AnimationFrame& a, AnimationFrame& b, float t, AnimationFrame& result)
    {
        uint numberOfTracks = a.Tracks.Size();
        result.Tracks.Resize(numberOfTracks);

        for (uint i = 0; i < numberOfTracks; ++i)
        {
            bool aActive = a.Tracks[i].Active;
            bool bActive = b.Tracks[i].Active;
            Any& valA = a.Tracks[i].Value;
            Any& valB = b.Tracks[i].Value;
            Any& dest = result.Tracks[i].Value;
            result.Tracks[i].Active = true;
            if (aActive & bActive)
            {
                if (valA.StoredType == LightningTypeId(Vec3))
                    dest = Math::Lerp(valA.Get<Vec3>(), valB.Get<Vec3>(), t);
                else if (valA.StoredType == LightningTypeId(Quat))
                    dest = Quat::SlerpUnnormalized(valA.Get<Quat>(), valB.Get<Quat>(), t);
                else
                    dest = valA;
            }
            else if (aActive)
            {
                dest = valA;
            }
            else
            {
                dest = valB;
            }
        }
    }

    struct AnimationLerpData
    {
        AnimationFrame& frame;
        float weight;
    };

    void LerpFrame(AnimationLerpData a, AnimationLerpData b, AnimationLerpData c, float t, AnimationFrame& result)
    {
        AnimationFrame& frameA = a.frame;
        AnimationFrame& frameB = b.frame;
        AnimationFrame& frameC = c.frame;

        uint numberOfTracks = frameA.Tracks.Size();
        result.Tracks.Resize(numberOfTracks);

        for (uint i = 0; i < numberOfTracks; ++i)
        {
            bool aActive = frameA.Tracks[i].Active;
            bool bActive = frameB.Tracks[i].Active;
            bool cActive = frameC.Tracks[i].Active;

            Any& valA = frameA.Tracks[i].Value;
            Any& valB = frameB.Tracks[i].Value;
            Any& valC = frameC.Tracks[i].Value;
            Any& dest = result.Tracks[i].Value;
            result.Tracks[i].Active = true;
            if (aActive & bActive & cActive)
            {

            }
            else if (aActive & cActive)
            {

            }
            else if (aActive & bActive)
            {
                if (valA.StoredType == LightningTypeId(Vec3))
                    dest = Math::Lerp(valA.Get<Vec3>(), valB.Get<Vec3>(), t);
                else if (valA.StoredType == LightningTypeId(Quat))
                    dest = Quat::SlerpUnnormalized(valA.Get<Quat>(), valB.Get<Quat>(), t);
                else
                    dest = valA;
            }

            else if (bActive & cActive)
            {

            }
            else if (aActive)
            {
                dest = valA;
            }
            else
            {
                dest = valB;
            }
        }
    }

    AnimationNode::AnimationNode()
    {
        mTime = 0.0f;
        mDuration = 0.0f;
        mTimeScale = 1.0f;
        mCollapseToPose = false;
        mCollapseToPoseOnFinish = true;
        mUpdatedFrameId = cInvalidFrameId;
    }

    LightningDefineType(AnimationNode, builder, type)
    {
        PlasmaBindDocumented();
        LightningBindMethod(Clone);
        LightningBindMethod(IsActive);
        LightningBindMethod(PrintNode);
        LightningBindMethod(CollapseToPose);
        LightningBindMethod(SetNormalizedTime);
        LightningBindMethod(GetNormalizedTime);
        LightningBindFieldProperty(mCollapseToPoseOnFinish);
        LightningBindFieldProperty(mPaused);
        LightningBindGetterSetterProperty(Time);
        LightningBindGetterSetterProperty(Duration);
        LightningBindFieldProperty(mTimeScale);
    }

    String AnimationNode::GetDisplayName()
    {
        return LightningVirtualTypeId(this)->Name;
    }

    void AnimationNode::CollapseToPose()
    {
        mCollapseToPose = true;
    }

    void AnimationNode::SetDuration(float duration)
    {
        mDuration = duration;
    }

    float AnimationNode::GetDuration()
    {
        return mDuration;
    }

    void AnimationNode::SetTime(float time)
    {
        mTime = Math::Clamp(time, 0.0f, mDuration);
    }

    float AnimationNode::GetTime()
    {
        return mTime;
    }

    void AnimationNode::SetNormalizedTime(float normalizedTime)
    {
        normalizedTime = Math::Clamp(normalizedTime, 0.0f, 1.0f);
        mTime = normalizedTime * mDuration;
    }

    float AnimationNode::GetNormalizedTime()
    {
        if (mDuration == 0.0f)
            return 0.0f;
        return mTime / mDuration;
    }

    bool AnimationNode::HasUpdatedThisFrame(uint frameId)
    {
        return mUpdatedFrameId == frameId;
    }

    bool AnimationNode::HasUpdatedAtLeastOnce()
    {
        return mUpdatedFrameId != cInvalidFrameId;
    }

    LightningDefineType(PoseNode, builder, type)
    {
    }

    PoseNode::PoseNode(AnimationFrame& pose)
    {
        mFrameData.Tracks.Assign(pose.Tracks.All());
    }

    AnimationNode* PoseNode::Update(AnimationGraph* animGraph, float dt, uint frameId, EventList eventsToSend)
    {
        return this;
    }

    void PoseNode::PrintNode(uint tabs)
    {
        PrintTabs(tabs);
        DebugPrint("Pose");
    }

    LightningDefineType(BasicAnimation, builder, type)
    {
        LightningBindGetterSetterProperty(Animation)->Add(new MetaEditorResource());
        LightningBindFieldProperty(mPlayMode);
    }

    BasicAnimation::BasicAnimation()
    {
        mDirection = 1.0f;
        mPlayMode = AnimationPlayMode::PlayOnce;
        mLoopCount = 0;
    }

    BasicAnimation::BasicAnimation(AnimationGraph* animGraph)
    {
        mDirection = 1.0f;
        mPlayMode = AnimationPlayMode::PlayOnce;
        mLoopCount = 0;
        mAnimGraph = animGraph;
    }

    BasicAnimation::BasicAnimation(AnimationGraph* animGraph,
                                   Animation* animation,
                                   float t,
                                   AnimationPlayMode::Enum playMode)
    {
        mDirection = 1.0f;
        mTime = t;
        mPlayMode = playMode;
        mLoopCount = 0;
        mAnimGraph = animGraph;
        SetAnimation(animation);
    }

    void BasicAnimation::ReLinkAnimations()
    {
        mPlayData.Clear();
        mDuration = 0.0f;
        if (mAnimation)
        {
            mDuration = mAnimation->mDuration;
            if (AnimationGraph* animGraph = mAnimGraph)
                animGraph->SetUpPlayData(mAnimation, mPlayData);
        }
    }

    AnimationNode* BasicAnimation::Update(AnimationGraph* animGraph, float dt, uint frameId, EventList eventsToSend)
    {
        // Return early if we've already been updated
        if (HasUpdatedThisFrame(frameId))
            return this;

        if (mCollapseToPose)
        {
            // If we haven't been updated yet, we need to pull the frame data from
            // the animation. Otherwise, we're just using last frames data
            if (HasUpdatedAtLeastOnce())
                UpdateFrame(animGraph);
            return new PoseNode(mFrameData);
        }

        // Update the frame id
        mUpdatedFrameId = frameId;

        Animation* animation = mAnimation;
        if (animation == nullptr)
            return this;

        mTime += dt * animGraph->GetTimeScale() * mDirection * mTimeScale;

        while ((mTime > mDuration && mDuration > 0.0f) || mTime < 0.0f)
        {
            AnimationGraphEvent e;
            e.mAnimation = mAnimation;
            e.mPlayMode = mPlayMode;
            e.mNode = this;

            // If we're playing once, we're done and can delete ourselves
            if (mPlayMode == AnimationPlayMode::PlayOnce)
            {
                // Queue an animation ended event
                e.EventId = Events::AnimationEnded;
                eventsToSend.PushBack(new AnimationGraphEvent(e));

                if (mCollapseToPoseOnFinish)
                {
                    // Clamp the time before updating the final frame
                    mTime = Math::Clamp(mTime, 0.0f, mDuration);

                    // Update the frame before collapsing
                    UpdateFrame(animGraph);

                    return new PoseNode(mFrameData);
                }
                else
                    return nullptr;
            }
            else
            {
                if (mPlayMode == AnimationPlayMode::Pingpong)
                {
                    // Don't go outside the range of the animation
                    mTime = Math::Clamp(mTime, 0.0f, mDuration);

                    // Flip the play direction
                    mDirection *= -1.0f;
                }
                else
                {
                    // Send animation looped event
                    if (mTime > mDuration)
                        mTime -= mDuration;
                    else
                        mTime += mDuration;
                }

                // Queue a loop event
                e.EventId = Events::AnimationLooped;
                eventsToSend.PushBack(new AnimationGraphEvent(e));
            }
        }

        // Update the frame with the animations data
        UpdateFrame(animGraph);

        return this;
    }

    void BasicAnimation::UpdateFrame(AnimationGraph* animGraph)
    {
        TrackParams params;
        params.Direction = TrackParams::Forward;
        params.Type = TrackParams::Game;
        params.Time = mTime;

        mFrameData.Tracks.Resize(animGraph->mBlendTracks.Size());
        mAnimation->UpdateFrame(mPlayData, params, mFrameData);
    }

    AnimationNode* BasicAnimation::Clone()
    {
        BasicAnimation* clone = new BasicAnimation(mAnimGraph);
        clone->mDuration = mDuration;
        clone->mTime = mTime;
        clone->mAnimation = mAnimation;
        clone->mPlayData = mPlayData;
        clone->mDirection = mDirection;
        return clone;
    }

    bool BasicAnimation::IsPlayingInNode(StringParam animName)
    {
        return animName == mAnimation->Name;
    }

    void BasicAnimation::PrintNode(uint tabs)
    {
        PrintTabs(tabs);
        String animName = mAnimation->Name;
        DebugPrint("BasicAnimation: %s, at: %.2gs of %.2gs\n", animName.c_str(), mTime, mDuration);
    }

    String BasicAnimation::GetDisplayName()
    {
        if (Animation* animation = mAnimation)
            return animation->Name;
        return AnimationNode::GetDisplayName();
    }

    Animation* BasicAnimation::GetAnimation()
    {
        return mAnimation;
    }

    void BasicAnimation::SetAnimation(Animation* animation)
    {
        if (animation == mAnimation)
            return;

        mAnimation = animation;

        ReLinkAnimations();
    }

    AnimationNode* BuildBasic(AnimationGraph* animGraph, Animation* animation, float t, AnimationPlayMode::Enum playMode)
    {
        return new BasicAnimation(animGraph, animation, t, playMode);
    }

    LightningDefineType(BlendSpaceData, builder, type)
    {
        LightningBindMethod(SetAnimationNode);
        LightningBindMethod(GetAnimationNode);
        LightningBindMethod(SetPosition);
        LightningBindMethod(GetPosition);
    }

     BlendSpaceData::BlendSpaceData() :
            mAnimationNode(nullptr),
            mPosition(Vec2(0,0))
    {

    }

    BlendSpaceData::BlendSpaceData(AnimationNode* animationNode, Vec2Param position) :
            mAnimationNode(animationNode),
            mPosition(position)
    {

    }

    BlendSpaceData::BlendSpaceData(const BlendSpaceData& data)
    {
        mAnimationNode = data.mAnimationNode;
        mPosition = data.mPosition;
    }

    bool BlendSpaceData::operator==(const BlendSpaceData& blendSpaceData) const
    {
        return mAnimationNode == blendSpaceData.mAnimationNode;
    }

    void BlendSpaceData::SetAnimationNode(AnimationNode* animationNode)
    {
        mAnimationNode = animationNode;
    }

    AnimationNode* BlendSpaceData::GetAnimationNode() const
    {
        return mAnimationNode;
    }

    void BlendSpaceData::SetPosition(Vec2Param position)
    {
        mPosition = position;
    }

    Vec2 BlendSpaceData::GetPosition() const
    {
        return mPosition;
    }

    LightningDefineType(BlendSpace1D, builder, type)
    {
        LightningBindMethod(AddAnimationData);
        LightningBindMethod(RemoveAnimationData);
        LightningBindMethod(CreateBlendSpaceData);

        LightningBindMethod(SetPosition);
        LightningBindMethod(GetPosition);
    }

    BlendSpace1D::BlendSpace1D()
    {

    }

    BlendSpace1D::BlendSpace1D(AnimationGraph* animGraph) : mAnimGraph(animGraph)
    {
    }

    BlendSpaceData* BlendSpace1D::CreateBlendSpaceData(AnimationNode* node, float position)
    {
        BlendSpaceData* blandData = new BlendSpaceData(node, Vec2(position, 0));
        return blandData;
    }

    void BlendSpace1D::AddAnimationData(BlendSpaceData* blendSpaceData)
    {
        if (!mAnimations.Contains(blendSpaceData))
        {
            mAnimations.PushBack(blendSpaceData);
            ReLinkAnimations();
        }
    }

    void BlendSpace1D::RemoveAnimationData(BlendSpaceData* blendSpaceData)
    {
        if (mAnimations.Contains(blendSpaceData))
        {
            int index = mAnimations.FindIndex(blendSpaceData);
            mAnimations.EraseAt(index);
            ReLinkAnimations();
        }
    }

    void BlendSpace1D::ReLinkAnimations()
    {
        for (BlendSpaceData* blendSpaceData : mAnimations)
        {
            AnimationNode* animationNode = blendSpaceData->GetAnimationNode();
            if (animationNode != nullptr)
            {
                animationNode->ReLinkAnimations();
            }
        }
    }

    void BlendSpace1D::SetPosition(float position)
    {
        mPosition = position;
    }
    float BlendSpace1D::GetPosition() const
    {
        return mPosition;
    }

    AnimationNode* BlendSpace1D::Update(AnimationGraph* animGraph, float dt, uint frameId, EventList eventsToSend)
    {
        return this;
    }

    AnimationNode* BlendSpace1D::Clone()
    {
        BlendSpace1D* clone = new BlendSpace1D();
        clone->mAnimations = mAnimations;
        clone->mPosition = mPosition;
        clone->mLastReturned = mLastReturned;
        clone->mAnimGraph = mAnimGraph;
        return clone;
    }

    bool BlendSpace1D::IsPlayingInNode(StringParam animName)
    {
        for (int i = 0; i < mAnimations.Size(); ++i)
        {
            if (mAnimations[i]->GetAnimationNode()->IsPlayingInNode(animName))
            {
                return true;
            }
        }
        return false;
    }

    void BlendSpace1D::PrintNode(uint tabs)
    {
    }

    String BlendSpace1D::GetDisplayName()
    {
        return "Unimplemented";
    }

    AnimationNode* BuildBlend1DSpace()
    {
        return new BlendSpace1D();
    }

    LightningDefineType(BlendSpace2D, builder, type)
    {
        LightningBindMethod(AddAnimationData);
        LightningBindMethod(RemoveAnimationData);
        LightningBindMethod(CreateBlendSpaceData);

        LightningBindMethod(SetPosition);
        LightningBindMethod(GetPosition);
    }

    BlendSpace2D::BlendSpace2D()
    {

    }

    BlendSpace2D::BlendSpace2D(AnimationGraph* animGraph) : mAnimGraph(animGraph)
    {
    }

    BlendSpaceData* BlendSpace2D::CreateBlendSpaceData(AnimationNode* node, Vec2Param position)
    {
        BlendSpaceData* blandData = new BlendSpaceData(node, position);
        return blandData;
    }

    void BlendSpace2D::AddAnimationData(BlendSpaceData* blendSpaceData)
    {
        if (!mAnimations.Contains(blendSpaceData))
        {
            mAnimations.PushBack(blendSpaceData);
            ReLinkAnimations();
        }
    }

    void BlendSpace2D::RemoveAnimationData(BlendSpaceData* blendSpaceData)
    {
        if (mAnimations.Contains(blendSpaceData))
        {
            int index = mAnimations.FindIndex(blendSpaceData);
            mAnimations.EraseAt(index);
            ReLinkAnimations();
        }
    }

    void BlendSpace2D::ReLinkAnimations()
    {
        for (BlendSpaceData* blendSpaceData : mAnimations)
        {
            AnimationNode* animationNode = blendSpaceData->GetAnimationNode();
            if (animationNode != nullptr)
            {
                animationNode->ReLinkAnimations();
            }
        }
    }

    void BlendSpace2D::SetPosition(Vec2Param position)
    {
        mPosition = position;
    }

    void BlendSpace2D::SetXPosition(float x)
    {
        mPosition.x = x;
    }

    void BlendSpace2D::SetYPosition(float y)
    {
        mPosition.y = y;
    }

    Vec2 BlendSpace2D::GetPosition() const
    {
        return mPosition;
    }

    AnimationNode* BlendSpace2D::Update(AnimationGraph* animGraph, float dt, uint frameId, EventList eventsToSend)
    {
        // Return early if we've already been updated
        if (HasUpdatedThisFrame(frameId))
        {
            if (AnimationNode* lastReturned = mLastReturned)
                return lastReturned;
            return this;
        }

        // Update the frame id
        mUpdatedFrameId = frameId;

        if (mCollapseToPose)
        {
            // Use the last frames data for the pose. If we haven't been updated yet,
            // we need to update with a dt of 0.0, then create the pose node
            if (HasUpdatedAtLeastOnce())
                return new PoseNode(mFrameData);

            // If we're collapsing to pose, we want to pull all animation data without
            // stepping forward in time
            dt = 0.0f;
        }

        mTime += dt * animGraph->GetTimeScale() * mTimeScale;

        // get 3 cloest points
        if (mAnimations.Size() >= 3)
        {
            // temporary data structure to help with blendspace distance checks
            struct NodeDistance
            {
                BlendSpaceData animation;
                float distance;
            };

            NodeDistance distnaceA;
            NodeDistance distnaceB;
            NodeDistance distnaceC;

            for (BlendSpaceData* blendSpaceData : mAnimations)
            {
                float distanceToData = Math::Distance(blendSpaceData->GetPosition(), mPosition);

                NodeDistance newNode;
                newNode.animation = *blendSpaceData;
                newNode.distance = distanceToData;

                // Waterfall data to we ensure we have 3 closest
                if (distanceToData <= distnaceA.distance)
                {
                    distnaceC = distnaceB;
                    distnaceB = distnaceA;
                    distnaceA = newNode;
                }
                else if (distanceToData <= distnaceB.distance)
                {
                    distnaceC = distnaceB;
                    distnaceB = newNode;
                }
                else if (distanceToData <= distnaceC.distance)
                {
                    distnaceC = newNode;
                }
            }

            // Use Barycentric coordinates to find the distance of mPosition from the 3 animation points
            Vec3 barycentricCoords;
            Geometry::BarycentricTriangle(Vec3(mPosition, 0), Vec3(distnaceA.animation.GetPosition()), Vec3(distnaceA.animation.GetPosition()), Vec3(distnaceC.animation.GetPosition(), 0), &barycentricCoords);

            AnimationNode* animationNodeA = distnaceA.animation.GetAnimationNode();
            AnimationNode* animationNodeB = distnaceB.animation.GetAnimationNode();
            AnimationNode* animationNodeC = distnaceC.animation.GetAnimationNode();

            // Update all animations before we use them for blending
            animationNodeA->Update(animGraph, dt, frameId, eventsToSend);
            animationNodeB->Update(animGraph, dt, frameId, eventsToSend);
            animationNodeC->Update(animGraph, dt, frameId, eventsToSend);

            AnimationFrame animationFrame;
            for (int i = 0; i < animationNodeA->mFrameData.Tracks.Size(); ++i)
            {
                animationFrame.Tracks[i].Value = animationNodeA->mFrameData.Tracks[i].Value.Get<Vec3>() * barycentricCoords.x;
                animationFrame.Tracks[i].Value = animationFrame.Tracks[i].Value.Get<Vec3>() + animationNodeB->mFrameData.Tracks[i].Value.Get<Vec3>() * barycentricCoords.y;
                animationFrame.Tracks[i].Value = animationFrame.Tracks[i].Value.Get<Vec3>() + animationNodeC->mFrameData.Tracks[i].Value.Get<Vec3>() * barycentricCoords.z;
            }
            return new PoseNode(animationFrame);
        }
        else
        {
            PlasmaPrint("Animation BlendSpace is invalid to to few animaitons");
            if (mAnimations.Empty())
            {
                Warn("No Animations in BlendSpace");
                return new PoseNode(mFrameData);
            }
            else
            {
                BlendSpaceData* blendSpaceData = mAnimations[0];
                if (AnimationNode* node = blendSpaceData->GetAnimationNode())
                {
                    node->Update(animGraph, dt, frameId, eventsToSend);
                    return new PoseNode(node->mFrameData);
                }
            }
        }



        // get barymetric coord
        // blend 3 animations
        // pose
    }

    AnimationNode* BlendSpace2D::Clone()
    {
        BlendSpace2D* clone = new BlendSpace2D();
        clone->mAnimations = mAnimations;
        clone->mPosition = mPosition;
        clone->mLastReturned = mLastReturned;
        clone->mAnimGraph = mAnimGraph;
        return clone;
    }

    bool BlendSpace2D::IsPlayingInNode(StringParam animName)
    {
        for (int i = 0; i < mAnimations.Size(); ++i)
        {
            if (mAnimations[i]->GetAnimationNode()->IsPlayingInNode(animName))
            {
                return true;
            }
        }
        return false;
    }

    void BlendSpace2D::PrintNode(uint tabs)
    {
    }

    String BlendSpace2D::GetDisplayName()
    {
        return "Unimplemented";
    }

    AnimationNode* BuildBlend2DSpace()
    {
        return new BlendSpace2D();
    }

    LightningDefineType(DirectBlend, builder, type)
    {
    }

    DirectBlend::DirectBlend()
    {
    }

    String DirectBlend::GetName()
    {
        return String("DirectBlend");
    }

    AnimationNode* DirectBlend::Update(AnimationGraph* animGraph, float dt, uint frameId, EventList eventsToSend)
    {
        // Return early if we've already been updated
        if (HasUpdatedThisFrame(frameId))
        {
            if (AnimationNode* lastReturned = mLastReturned)
                return lastReturned;
            return this;
        }

        // Update the frame id
        mUpdatedFrameId = frameId;

        if (mCollapseToPose)
        {
            // Use the last frames data for the pose. If we haven't been updated yet,
            // we need to update with a dt of 0.0, then create the pose node
            if (HasUpdatedAtLeastOnce())
                return new PoseNode(mFrameData);

            // If we're collapsing to pose, we want to pull all animation data without
            // stepping forward in time
            dt = 0.0f;
        }

        mTime += dt * animGraph->GetTimeScale() * mTimeScale;

        // If the blend is done, return the right branch
        if (mTime > mDuration)
        {
            mLastReturned = mB;
            return CollapseToB(animGraph, frameId, eventsToSend);
        }

        float t = mTime / mDuration;

        // Update the left branch
        mA = mA->Update(animGraph, 0, frameId, eventsToSend);
        if (mA.IsNull())
        {
            mLastReturned = mB;
            return CollapseToB(animGraph, frameId, eventsToSend);
        }

        // Update the right branch
        mB = mB->Update(animGraph, 0, frameId, eventsToSend);
        if (mB.IsNull())
        {
            mLastReturned = mA;
            return CollapseToA(animGraph, frameId, eventsToSend);
        }

        // Interpolate between the two branches
        LerpFrame(mA->mFrameData, mB->mFrameData, t, mFrameData);

        // Now that we've updated our frame data, we can create the pose node
        if (mCollapseToPose)
            return new PoseNode(mFrameData);

        return this;
    }

    void DirectBlend::PrintNode(uint tabs)
    {
        PrintTabs(tabs);
        DebugPrint("Direct-Blend between:\n");
        mA->PrintNode(tabs + 1);
        mB->PrintNode(tabs + 1);
    }

    AnimationNode* BuildDirectBlend(AnimationGraph* t, AnimationNode* a, AnimationNode* b, float transitionTime)
    {
        DirectBlend* direct = new DirectBlend();
        direct->mA = a;
        direct->mB = b;
        direct->SetTime(0);
        direct->SetDuration(transitionTime);
        return direct;
    }

    LightningDefineType(CrossBlend, builder, type)
    {
        LightningBindFieldProperty(mTimeScaleFrom);
        LightningBindFieldProperty(mTimeScaleTo);
        LightningBindFieldProperty(mType);
        LightningBindFieldProperty(mMode);
        LightningBindMethod(SyncCadence);
        LightningBindMethod(SetNormalizedTimeScale);
    }

    CrossBlend::CrossBlend()
    {
        mTimeScaleFrom = 1.0f;
        mTimeScaleTo = 1.0f;
        mType = AnimationBlendType::Standard;
        mMode = AnimationBlendMode::Auto;
    }

    String CrossBlend::GetName()
    {
        return String("CrossBlend");
    }

    AnimationNode* CrossBlend::Update(AnimationGraph* animGraph, float dt, uint frameId, EventList eventsToSend)
    {
        // Return early if we've already been updated
        if (HasUpdatedThisFrame(frameId))
        {
            if (AnimationNode* lastReturned = mLastReturned)
                return lastReturned;
            return this;
        }

        if (mCollapseToPose)
        {
            // Use the last frames data for the pose. If we haven't been updated yet,
            // we need to update with a dt of 0.0, then create the pose node
            if (HasUpdatedAtLeastOnce())
                return new PoseNode(mFrameData);

            // If we're collapsing to pose, we want to pull all animation data without
            // stepping forward in time
            dt = 0.0f;
        }

        // Update the frame id
        mUpdatedFrameId = frameId;

        // Step forward if we're in auto mode
        if (mMode == AnimationBlendMode::Auto)
        {
            float timeScale = animGraph->GetTimeScale() * mTimeScale;
            mTime += dt * timeScale;
        }

        // The normalized time for this blend
        float blendT = 0.0f;
        if (mDuration != 0.0f)
            blendT = mTime / mDuration;

        // Cross blend will also cross blend the animation speed
        if (mMode == AnimationBlendMode::Auto)
        {
            // If the blend is done, return the right branch
            if (mTime > mDuration)
            {
                mLastReturned = mB;
                return CollapseToB(animGraph, frameId, eventsToSend);
            }
        }

        if (mType == AnimationBlendType::Normalized)
        {
            float rateA = 0.0f;
            if (mA->GetDuration() > 0.0f)
                rateA = 1.0f / mA->GetDuration();

            float rateB = 0.0f;
            if (mB->GetDuration() > 0.0f)
                rateB = 1.0f / mB->GetDuration();

            float percentagePassed = Math::Lerp(rateA, rateB, blendT);

            mTimeScaleFrom = percentagePassed * mA->GetDuration();
            mTimeScaleTo = percentagePassed * mB->GetDuration();
        }

        // Update the left branch
        mA = mA->Update(animGraph, dt * mTimeScaleFrom, frameId, eventsToSend);
        if (!mA)
        {
            mLastReturned = mB;
            return CollapseToB(animGraph, frameId, eventsToSend);
        }

        // Update the right branch
        mB = mB->Update(animGraph, dt * mTimeScaleTo, frameId, eventsToSend);
        if (!mB)
        {
            mLastReturned = mA;
            return CollapseToA(animGraph, frameId, eventsToSend);
        }

        // Interpolate between the two branches
        LerpFrame(mA->mFrameData, mB->mFrameData, blendT, mFrameData);

        // Now that we've updated our frame data, we can create the pose node
        if (mCollapseToPose)
            return new PoseNode(mFrameData);

        return this;
    }

    void CrossBlend::PrintNode(uint tabs)
    {
        PrintTabs(tabs);
        DebugPrint("Cross-Blend between:\n");
        mA->PrintNode(tabs + 1);
        mB->PrintNode(tabs + 1);
    }

    void CrossBlend::SyncCadence()
    {
        float normalizedT = 0.0f;
        if (mA->GetDuration() > 0.0f)
            normalizedT = mA->GetTime() / mA->GetDuration();
        mB->SetTime(normalizedT * mB->GetDuration());
    }

    void CrossBlend::SetNormalizedTimeScale(float min, float max, float current)
    {
        mType = AnimationBlendType::Standard;

        float rateA = 0.0f;
        if (mA->GetDuration() > 0.0f)
            rateA = 1.0f / mA->GetDuration();

        float rateB = 0.0f;
        if (mB->GetDuration() > 0.0f)
            rateB = 1.0f / mB->GetDuration();

        float blendTime = (current - min) / (max - min);

        if (blendTime < 0.0f)
        {
            mTimeScaleFrom = current / min;
            mTimeScaleTo = mB->GetDuration() * mTimeScaleFrom * rateA;
        }
        else if (blendTime > 1.0f)
        {
            mTimeScaleTo = current / max;
            mTimeScaleFrom = mA->GetDuration() * mTimeScaleTo * rateB;
        }
        else
        {
            float rate = Math::Lerp(rateA, rateB, blendTime);
            mTimeScaleFrom = mA->GetDuration() * rate;
            mTimeScaleTo = mB->GetDuration() * rate;
        }

        SetNormalizedTime(Math::Clamp(blendTime, 0.0f, 1.0f));
    }

    AnimationNode* BuildCrossBlend(AnimationGraph* t, AnimationNode* a, AnimationNode* b, float transitionTime)
    {
        CrossBlend* blend = new CrossBlend();

        blend->mA = a;
        blend->mB = b;

        blend->SyncCadence();

        blend->SetTime(0);
        blend->SetDuration(transitionTime);

        return blend;
    }

    LightningDefineType(SelectiveNode, builder, type)
    {
        LightningBindGetterSetterProperty(Root);
    }

    SelectiveNode::SelectiveNode()
    {
    }

    String SelectiveNode::GetName()
    {
        return String("SelectiveNode");
    }

    AnimationNode* SelectiveNode::Update(AnimationGraph* animGraph, float dt, uint frameId, EventList eventsToSend)
    {
        // Return early if we've already been updated
        if (HasUpdatedThisFrame(frameId))
        {
            if (AnimationNode* lastReturned = mLastReturned)
                return lastReturned;
            return this;
        }

        // Update the frame id
        mUpdatedFrameId = frameId;

        if (mCollapseToPose)
        {
            // Use the last frames data for the pose. If we haven't been updated yet,
            // we need to update with a dt of 0.0, then create the pose node
            if (HasUpdatedAtLeastOnce())
                return new PoseNode(mFrameData);

            // If we're collapsing to pose, we want to pull all animation data without
            // stepping forward in time
            dt = 0.0f;
        }

        // Update the left branch
        if (mA)
            mA = mA->Update(animGraph, dt * mTimeScale, frameId, eventsToSend);

        // Update the right branch
        mB = mB->Update(animGraph, dt * mTimeScale, frameId, eventsToSend);
        if (!mB)
        {
            mLastReturned = mA;
            return CollapseToA(animGraph, frameId, eventsToSend);
        }

        // Overwrite the 'A' track values with 'B' track values
        AnimationFrame& frameA = mA->mFrameData;
        AnimationFrame& frameB = mB->mFrameData;

        uint trackCount = frameA.Tracks.Size();
        mFrameData.Tracks.Resize(trackCount);
        for (uint i = 0; i < trackCount; ++i)
        {
            Any& dest = mFrameData.Tracks[i].Value;
            mFrameData.Tracks[i].Active = true;

            if (mSelectiveBones.FindValue((int)i, uint(-1)) != uint(-1))
            {
                Any& valB = frameB.Tracks[i].Value;
                dest = valB;
            }
            else if (mA)
            {
                Any& valA = frameA.Tracks[i].Value;
                dest = valA;
            }
        }

        // Now that we've updated our frame data, we can create the pose node
        if (mCollapseToPose)
            return new PoseNode(mFrameData);

        return this;
    }

    AnimationNode* SelectiveNode::Clone()
    {
        SelectiveNode* clone = new SelectiveNode();
        clone->mDuration = mDuration;
        clone->mTime = mTime;
        clone->mA = mA->Clone();
        clone->mB = mB->Clone();
        clone->mSelectiveBones = mSelectiveBones;
        return clone;
    }

    void SelectiveNode::PrintNode(uint tabs)
    {
        PrintTabs(tabs);
        DebugPrint("Isolated-Blend between:\n");
        mA->PrintNode(tabs + 1);
        mB->PrintNode(tabs + 1);
    }

    void GetChildIndices(Cog* object, AnimationGraph* t, HashSet<uint>& indices)
    {
        // Walk through the blend tracks and find anything with this object
        BlendTracks::range r = t->mBlendTracks.All();
        for (; !r.Empty(); r.PopFront())
        {
            BlendTrack* track = r.Front().second;
            Transform* currObject = track->Object.Get<Transform*>();
            if (currObject == nullptr)
                continue;
            if (currObject->GetOwner() == object)
                indices.Insert(track->Index);
        }

        // Recursively call each child of the object
        Hierarchy* hierarchy = object->has(Hierarchy);
        if (hierarchy)
        {
            HierarchyList::range range = hierarchy->GetChildren();
            for (; !range.Empty(); range.PopFront())
                GetChildIndices(&range.Front(), t, indices);
        }
    }

    AnimationGraph* GetAnimationGraph(Cog* object)
    {
        if (object == nullptr)
            return nullptr;
        if (AnimationGraph* animGraph = object->has(AnimationGraph))
            return animGraph;
        return GetAnimationGraph(object->GetParent());
    }

    void SelectiveNode::SetRoot(Cog* root)
    {
        mRoot = root;
        AnimationGraph* animGraph = GetAnimationGraph(root);
        GetChildIndices(root, animGraph, mSelectiveBones);
    }

    Cog* SelectiveNode::GetRoot()
    {
        return mRoot;
    }

    AnimationNode* BuildSelectiveNode(AnimationGraph* t, AnimationNode* a, AnimationNode* b, Cog* rootBone)
    {
        SelectiveNode* selective = new SelectiveNode();

        selective->mA = a;
        selective->mB = b;
        selective->SetRoot(rootBone);

        return selective;
    }

    LightningDefineType(ChainNode, builder, type)
    {
    }

    ChainNode::ChainNode()
    {
    }

    String ChainNode::GetName()
    {
        return String("ChainNode");
    }

    AnimationNode* ChainNode::Update(AnimationGraph* animGraph, float dt, uint frameId, EventList eventsToSend)
    {
        // Return early if we've already been updated
        if (HasUpdatedThisFrame(frameId))
        {
            if (AnimationNode* lastReturned = mLastReturned)
                return lastReturned;
            return this;
        }

        if (mCollapseToPose)
        {
            // Use the last frames data for the pose. If we haven't been updated yet,
            // we need to update with a dt of 0.0, then create the pose node
            if (HasUpdatedAtLeastOnce())
                return new PoseNode(mFrameData);

            // If we're collapsing to pose, we want to pull all animation data without
            // stepping forward in time
            dt = 0.0f;
        }

        // Update the frame id
        mUpdatedFrameId = frameId;

        if (mA == nullptr)
            return this;

        // Keep around a reference so that it cannot be deleted in the update
        HandleOf<AnimationNode> tempRefA = mA;
        mA = mA->Update(animGraph, dt, frameId, eventsToSend);
        if (!mA)
        {
            mLastReturned = mB;
            return CollapseToB(animGraph, frameId, eventsToSend);
        }

        // Copy over tracks from the current child. This should be optimized
        mFrameData.Tracks.Assign(mA->mFrameData.Tracks.All());

        // Now that we've updated our frame data, we can create the pose node
        if (mCollapseToPose)
            return new PoseNode(mFrameData);

        return this;
    }

    bool ChainNode::IsPlayingInNode(StringParam animName)
    {
        return mA->IsPlayingInNode(animName) || mB->IsPlayingInNode(animName);
    }

    void ChainNode::PrintNode(uint tabs)
    {
        PrintTabs(tabs);
        DebugPrint("Chain A then B:\n");
        mA->PrintNode(tabs + 1);
        mB->PrintNode(tabs + 2);
    }

    AnimationNode* BuildChainNode(AnimationGraph* t, AnimationNode* a, AnimationNode* b)
    {
        ChainNode* chain = new ChainNode();
        chain->mA = a;
        chain->mB = b;
        return chain;
    }

} // namespace Plasma
