package com.viro.renderer.jni;


import java.lang.ref.WeakReference;

/**
 * The PhysicsWorld encapsulates the physics simulation associated with the {@link SceneController}.
 * Each Scene automatically creates a PhysicsWorld upon construction.
 * This object can be used to set global physics properties like gravity.
 */
public class PhysicsWorld {

    /**
     * Callback used when hit tests are completed.
     */
    public interface HitTestCallback{
        void onComplete(boolean hasHit);
    }

    private WeakReference<SceneController> mScene;

    PhysicsWorld(SceneController scene) {
        mScene = new WeakReference<SceneController>(scene);
    }

    public void setGravity(Vector gravity) {
        SceneController scene = mScene.get();
        if (scene != null) {
            scene.setPhysicsWorldGravity(gravity.toArray());
        }
    }

    public void setDebugDraw(boolean debugDraw) {
        SceneController scene = mScene.get();
        if (scene != null) {
            scene.setPhysicsDebugDraw(debugDraw);
        }
    }

    public void findCollisionsWithRayAsync(Vector from, Vector to, boolean closest,
                                           String tag, HitTestCallback callback) {
        SceneController scene = mScene.get();
        if (scene != null) {
            scene.findCollisionsWithRayAsync(from.toArray(), to.toArray(), closest, tag, callback);
        }
    }

    public void findCollisionsWithShapeAsync(Vector from, Vector to, PhysicsShape shape,
                                             String tag, HitTestCallback callback) {
        SceneController scene = mScene.get();
        if (scene != null) {
            scene.findCollisionsWithShapeAsync(from.toArray(), to.toArray(), shape.getType(), shape.getParams(), tag, callback);
        }
    }

}
