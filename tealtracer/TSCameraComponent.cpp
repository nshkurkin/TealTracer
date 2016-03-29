//
//  TSCameraComponent.cpp
//  VitalK
//
//  Created by Nikolai Shkurkin on 3/25/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "TSCameraComponent.hpp"

#include "TSGameObject.hpp"
#include "TSServiceProvider.hpp"
#include "TSTRansformComponent.hpp"

///
///
TSCameraComponent::Projection::Projection() : ortho(false), aspect(1.0f), fovy(45.0f / 180.0f * (float) M_PI), left(-1.0f), right(1.0f), bottom(-1.0f), top(1.0f), znear(0.1f), zfar(1000.0f) {
     
}

///
const TSCameraComponent::Projection &
TSCameraComponent::projection() const {
    return projection_;
}

///
void
TSCameraComponent::setProjection(const TSCameraComponent::Projection & projection) {
    projection_ = projection;
    projectionMatrixDirty_ = true;
}

///
const glm::mat4x4 &
TSCameraComponent::cachedViewMatrix() const {
    return viewMatrix_;
}

///
const glm::mat4x4 &
TSCameraComponent::cachedInverseViewMatrix() const {
    return invViewMatrix_;
}

///
const glm::mat4x4 &
TSCameraComponent::cachedProjectionMatrix() const {
    return projectionMatrix_;
}

///
const glm::mat4x4 &
TSCameraComponent::cachedViewProjectionMatrix() const {
    return viewProjectionMatrix_;
}

///
void
TSCameraComponent::updateWillBegin(TSGameObject * obj, TSServiceProvider & services) {
    if (obj->transformComponent().localTransformHasChanged()) {
        viewMatrix_ = obj->transformComponent().transform();
        viewMatrixDirty_ = true;
        flushChanges();
    }
}

///
void
TSCameraComponent::update(TSGameObject * obj, TSServiceProvider & services) {
    
}

///
void
TSCameraComponent::updateDone(TSGameObject *obj, TSServiceProvider & services) {
    flushChanges();
}

///
void
TSCameraComponent::flushChanges() {
    updateViewProjectionMatrix();
    
    projection_.swap();
    projectionMatrix_.swap();
    viewMatrix_.swap();
    invViewMatrix_.swap();
    viewProjectionMatrix_.swap();
    
    projectionMatrixDirty_.overwrite(false);
    viewMatrixDirty_.overwrite(false);
}

///
void
TSCameraComponent::updateViewProjectionMatrix() {
    bool wasDirty = viewMatrixDirty_.nextValue() || projectionMatrixDirty_.nextValue();
    if (viewMatrixDirty_.nextValue()) {
        invViewMatrix_ = glm::inverse(viewMatrix_.nextValue());
        viewMatrixDirty_ = false;
    }
    if (projectionMatrixDirty_.nextValue()) {
        if (projection_->ortho) {
            projectionMatrix_ = glm::ortho(projection_->left, projection_->right, projection_->bottom, projection_->top, projection_->znear, projection_->zfar);
        }
        else {
            projectionMatrix_ = glm::perspective(projection_->fovy, projection_->aspect, projection_->znear, projection_->zfar);
        }
        projectionMatrixDirty_ = false;
    }
    if (wasDirty) {
        viewProjectionMatrix_ = projectionMatrix_.nextValue() * invViewMatrix_.nextValue();
    }
}


