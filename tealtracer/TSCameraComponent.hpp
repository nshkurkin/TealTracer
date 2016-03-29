//
//  TSCameraComponent.hpp
//  VitalK
//
//  Created by Nikolai Shkurkin on 3/25/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef TSCameraComponent_hpp
#define TSCameraComponent_hpp

#include "TSComponent.hpp"
#include "TSDoubleBufferedValue.hpp"

class TSCameraComponent : public TSComponent {
public:

    ///
    struct Projection {
        bool ortho;
        float aspect;
        float fovy;
        float left, right, bottom, top;
        float znear, zfar;

        Projection();
    };
    
    ///
    const Projection & projection() const;
    ///
    void setProjection(const Projection & projection);
    
    ///
    const glm::mat4x4 & cachedViewMatrix() const;
    
    ///
    const glm::mat4x4 & cachedInverseViewMatrix() const;
    ///
    const glm::mat4x4 & cachedProjectionMatrix() const;
    ///
    const glm::mat4x4 & cachedViewProjectionMatrix() const;

protected:
    friend class TSRenderEngine;
    ///
    virtual void updateWillBegin(TSGameObject * obj, TSServiceProvider & services);
    ///
    virtual void update(TSGameObject * obj, TSServiceProvider & services);
    ///
    virtual void updateDone(TSGameObject *obj, TSServiceProvider & services);

private:
    
    ///
    void updateViewProjectionMatrix();
    
private:
    ///
    void flushChanges();

    ///
    TSDoubleBufferedValue<Projection> projection_;
    
    ///
    TSDoubleBufferedValue<glm::mat4x4> projectionMatrix_;
    ///
    TSDoubleBufferedValue<glm::mat4x4> viewMatrix_;
    ///
    TSDoubleBufferedValue<glm::mat4x4> invViewMatrix_;
    ///
    TSDoubleBufferedValue<glm::mat4x4> viewProjectionMatrix_;
    
    ///
    TSDoubleBufferedValue<bool> projectionMatrixDirty_;
    ///
    TSDoubleBufferedValue<bool> viewMatrixDirty_;

};

#endif /* TSCameraComponent_hpp */
