///
/// MatStack.h
/// ----------
/// Nikolai Shkurkin
/// Utility Library
///
/// Notes: Adapted from Dr. Sueda's "MatrixStack" code.
///

#ifndef ____util_MatStack__
#define ____util_MatStack__

#include <stack>
#include <Eigen/Dense>

#include <src/util/util_base.h>
#include <src/util/util_types.h>

namespace util {
    
    /// MatStack is an abstraction of a matrix stack used in graphics for things
    /// like hierarchical modeling. It is generic to allow for arbitrary types
    /// of matrix stacks (such as floating point precision and double),
    /// especially when using Google's Ceres solver (which requires a Jet<T>
    /// type).
    template <typename T>
    class MatStack {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        DEFINE_EIGEN_TYPES(T)
        
        /// Creates a Matrix Stack with the identity on top.
        MatStack() {
            mstack.push(Mat4::Identity());
        }
        
        virtual ~MatStack() {}
        
        /// Duplicates the `top` matrix and places it on top of the stack
        void push() {
            const Mat4 &top = mstack.top();
            mstack.push(top);
        }
        
        /// Pops off the `top` matrix
        void pop() {
            mstack.pop();
        }
        
        /// The top of the stack is set to the identity matrix
        void loadIdentity() {
            Mat4 & top = mstack.top();
            top = Mat4::Identity();
        }
        
        /// Multiplies the `top` matrix by `matrix`
        void multMatrix(const Mat4 &matrix) {
            Mat4 &top = mstack.top();
            top *= matrix;
        }
        
        /// Applies a translation matrix to `top`
        void translate(const Vec3 &trans) {
            Mat4 &top = mstack.top();
            Mat4 E = Mat4::Identity();
            E(0,3) = trans(0);
            E(1,3) = trans(1);
            E(2,3) = trans(2);
            top *= E;
        }
        
        /// Applies a scaling matrix to `top`
        void scale(const Vec3 &scale) {
            Mat4 &top = mstack.top();
            Mat4 E = Mat4::Identity();
            E(0,0) = scale(0);
            E(1,1) = scale(1);
            E(2,2) = scale(2);
            top *= E;
        }
        
        /// Applies a scaling matrix to `top`
        void scale(T s) {
            Mat4 &top = mstack.top();
            Mat4 E = Mat4::Identity();
            E(0,0) = s;
            E(1,1) = s;
            E(2,2) = s;
            top *= E;
        }
        
        /// Applies an Axis Angle rotation to `top`
        void rotate(T angle, const Vec3 &axis) {
            Mat4 &top = mstack.top();
            Mat4 E = Mat4::Identity();
            E.block(0, 0, 3, 3) = AngleAxis(angle, axis).toRotationMatrix();
            top *= E;
        }
        
        /// Applies a quaternion rotation to `top`
        void rotate(Quat quat) {
            Mat4 mat4 = Mat4::Identity();
            mat4.block(0, 0, 3, 3) = quat.matrix();
            multMatrix(mat4);
        }
        
        /// Retrieves the top matrix on the stack
        Mat4 & top() {
            return mstack.top();
        }
        
        /// Set the top matrix to an orthographic projection matrix
        void ortho(T left, T right, T bottom, T top, T zNear, T zFar) {
            assert(left != right);
            assert(bottom != top);
            assert(zFar != zNear);
            // Sets the top of the stack
            Mat4 &M = mstack.top();
            M = Mat4::Zero();
            M(0,0) = T(2.0) / (right - left);
            M(1,1) = T(2.0) / (top - bottom);
            M(2,2) = T(-2.0) / (zFar - zNear);
            M(0,3) = - (right + left) / (right - left);
            M(1,3) = - (top + bottom) / (top - bottom);
            M(2,3) = - (zFar + zNear) / (zFar - zNear);
            M(3,3) = T(1.0);
        }
        
        /// Sets the top matrix to a perspective matrix
        void perspective(T fovy, T aspect, T zNear, T zFar) {
            assert(fovy != T(0.0));
            assert(aspect != T(0.0));
            assert(zFar != zNear);
            // Sets the top of the stack
            Mat4 &M = mstack.top();
            M = Mat4::Zero();
            T tanHalfFovy = tan(T(0.5) * fovy);
            M(0,0) = T(1.0) / (aspect * tanHalfFovy);
            M(1,1) = T(1.0) / (tanHalfFovy);
            M(2,2) = -(zFar + zNear) / (zFar - zNear);
            M(2,3) = -(T(2.0) * zFar * zNear) / (zFar - zNear);
            M(3,2) = -T(1.0);
        }
        
        /// Returns the result of applying `vec` with a 0.0 as the homogenous
        /// coordinate. This prevents translations from taking effect on the
        /// vector.
        //template < int N >
        //Eigen::Matrix<T, N, 1> applyToVec(Eigen::Matrix<T, N, 1> vec) {
        //    Eigen::Matrix<T, 4, 1> vec4(T(0), T(0), T(0), T(0));
        //    vec4.segment(0, N) = vec.segment(0, N);
        //    vec4 = top() * vec4;
        //    return Eigen::Matrix<T, N, 1>(vec4.segment(0, N));
        //}

        Eigen::Matrix<T, 2, 1> applyToVec(Eigen::Matrix<T, 2, 1> vec) {
            Eigen::Matrix<T, 4, 1> vec4(T(0), T(0), T(0), T(0));
            vec4.segment(0, 2) = vec.segment(0, 2);
            vec4 = top() * vec4;
            return Eigen::Matrix<T, 2, 1>(vec4.segment(0, 2));
        }

        Eigen::Matrix<T, 3, 1> applyToVec(Eigen::Matrix<T, 3, 1> vec) {
            Eigen::Matrix<T, 4, 1> vec4(T(0), T(0), T(0), T(0));
            vec4.segment(0, 3) = vec.segment(0, 3);
            vec4 = top() * vec4;
            return Eigen::Matrix<T, 3, 1>(vec4.segment(0, 3));
        }

        Eigen::Matrix<T, 4, 1> applyToVec(Eigen::Matrix<T, 4, 1> vec) {
            return top() * vec;
        }
        
        /// Returns the result of applying `vec` with a 1.0 as the homogenous
        /// coordinate. This allows translations to take effect on the
        /// position.
		template < int N >
		Eigen::Matrix<T, N, 1> applyToPos(Eigen::Matrix<T, N, 1> pos) {
			Eigen::Matrix<T, 4, 1> vec4(T(0), T(0), T(0), T(1));
			vec4.segment(0, N) = pos.segment(0, N);
			vec4 = top() * vec4;
			return Eigen::Matrix<T, N, 1>(vec4.segment(0, N));
		}

		util::Vec3f applyToPos(util::Vec3f pos) {
			util::Vec4f vec4(0, 0, 0, 1);
			vec4.segment(0, 3) = pos.segment(0, 3);
			vec4 = top() * vec4;
			return util::Vec3f(vec4.segment(0, 3));
		}
        
    private:
        /// Underlying matrix stack
        std::stack< Mat4 > mstack;
    };
    
    typedef MatStack<float> MatStackf;
    /// A floating point matrix stack.
    typedef MatStackf MatrixStack;
}

#endif // ____util_MatStack__

