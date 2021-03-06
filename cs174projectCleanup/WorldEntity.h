#ifndef _WORLDENTITY
#define _WORLDENTITY

#include "Angel.h"

/** @brief This class provides objects with a position, rotation, scale, and
 *         parentage.
 *  Most, if not all, distinct classes involved with rendering (i.e., rendered
 *  objects or the camera) will derive from this class.
 * 
 *  This class, while not technically abstract, should not be instantiated
 *  directly.
 */
class WorldEntity{
private:
	vec3 _position; ///< The position of the object relative to the parent.
	vec3 _rotation; ///< The rotation of the object relative to the parent.
	vec3 _scale; ///< The scale of the object relative to the parent.
	
	//Mutable means that a const member function may change it.
	//They can only be used for optimization purposes (like caching or memoization)
	//and cannot ultimately affect the external behavior of the class.
	mutable vec3 _worldPosition; ///< The last calcuated world position.
	mutable unsigned int _positionFrameCount; ///< The frame count at the last time position was calculated.

	const WorldEntity* _parent; ///< The object's parent or NULL if none.

public:
	/** @brief Constructs a new WorldEntity object.
	 *  @param parent The object's parent or NULL if none.
	 *   The object's local transformations are concatenated with
	 *   its ancestors' before finally being rendered.
	 */
	WorldEntity(const WorldEntity* parent = NULL);
	virtual ~WorldEntity() {}

	//Translation
	/** @brief Translates the object in addition to any previous
	 *         translations.
	 */
	void translate(vec3 v);
	/** @overload void translate(vec3 v) */
	void translate(float x, float y, float z);
	/** @brief Sets the local position, disregarding any previous local
	 *         translations.
	 */
	void setTranslate(float x, float y, float z);
	/** @brief Sets the local X position, disregarding any previous local
	 *         translations.
	 */
	void setTranslate(vec3 v);
	void setTranslateX(float x);
	/** @brief Sets the local Y position, disregarding any previous local
	 *         translations.
	 */
	void setTranslateY(float y);
	/** @brief Sets the local Z position, disregarding any previous local
	 *         translations.
	 */
	void setTranslateZ(float z);
	/** @brief Gets the total translation vector. 
	  * @return The local translation vector concatenated with the
	  *         ancestors'.
	  */
	vec3 getTranslate() const;
	/** @brief Gets the local translation vector. */
	vec3 getLocalTranslate() const { return _position; }

	//Rotation
	/** @brief Rotates the object in addition to any previous
	 *         rotations.
	 */
	void rotate(float x, float y, float z);
	/** @brief Sets the local rotation, disregarding any previous local
	 *         rotations.
	 */
	void setRotate(float x, float y, float z);
	/** @brief Gets the total rotation vector. 
	  * @return The local rotation vector concatenated with the
	  *         ancestors'.
	  */
	vec3 getRotate() const;
	/** @brief Gets the local rotation vector. */
	vec3 getLocalRotate() const { return _rotation; }

	//Scale
	/** @brief Scales the object in addition to any previous
	 *         scaling.
	 */
	void scale(float x, float y, float z);
	/** @brief Sets the local scale, disregarding any previous local
	 *         scaling.
	 */
	void setScale(float x, float y, float z);
	/** @brief Gets the total scale vector. 
	  * @return The local scale vector concatenated with the
	  *         ancestors'.
	  */
	vec3 getScale() const;
	/** @brief Gets the local scale vector. */
	vec3 getLocalScale()const { return _scale; }

	//Parent
	/** @brief Returns the parent object or NULL if none. */
	inline const WorldEntity* getParent() const { return _parent; }
	inline void setParent(const WorldEntity* p){_parent=p;}


	//TransformationMatrix
	/** @brief Computes the total transformation matrix by concatenating
	 *         this object's translation, rotation, and scale operations
	 *         with those of its ancestors.
	 */
	mat4 getTransformationMatrix() const;

	//TransformationMatrix
	/** @brief Computes the reverse transformation for the camera. Does
	 *         Does not use scale modifier and takes into account traslation and rotation
	 *         of its ancestors.
	 */
	mat4 getCameraTransformationMatrix() const;
};


#endif //_WORLDENTITY