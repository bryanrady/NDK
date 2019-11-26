#include <stdint.h>

//这个是6.0源码的art 所以运行在7.0以上可能会报错

namespace art{
    namespace mirror{
        class Object{
            // The Class representing the type of the object.
            uint32_t klass_;
            // Monitor and hash code information.
            uint32_t monitor_;

        };
        class ArtMethod:public Object{
        public:
            // Access flags; low 16 bits are defined by spec.
            uint32_t access_flags_;

            /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */
            // Offset to the CodeItem.
            uint32_t dex_code_item_offset_;
            // Index into method_ids of the dex file associated with this method.
            uint32_t dex_method_index_;
            /* End of dex file fields. */
            // Entry within a dispatch table for this method. For static/direct methods the index is into
            // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
            // ifTable.
            uint32_t method_index_;

            // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
            uint32_t dex_cache_resolved_methods_;

            // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
            uint32_t dex_cache_resolved_types_;


            // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
            // The class we are a part of.
            uint32_t declaring_class_;
        };
    }
}