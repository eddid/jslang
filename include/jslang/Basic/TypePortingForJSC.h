/**
 * Why needs this file:
 * I use flex and bison to generate lexical and syntax parser in the first stage.
 * I modify ecmascript.y base on  WebKit/JavaScriptCore/parser/Grammy.y(yes, they drop it later).
 * I have to use AST nodes types in WebKit/JavaScriptCore/parser/Nodes.h since I don't want to rewrite it.
 * And this file is the poring layer of JavaScriptCore files I use
 */
#ifndef TYPEPORTINGFORJSC_H
#define TYPEPORTINGFORJSC_H

#include <stdlib.h>
#include <llvm/IR/Value.h>

/**
 * MACRO PORTING PART
 */
#if 1
#define COMPILER(WTF_FEATURE) (defined WTF_COMPILER_##WTF_FEATURE  && WTF_COMPILER_##WTF_FEATURE)
#define CPU(WTF_FEATURE) (defined WTF_CPU_##WTF_FEATURE  && WTF_CPU_##WTF_FEATURE)
#define USE(WTF_FEATURE) (defined WTF_USE_##WTF_FEATURE  && WTF_USE_##WTF_FEATURE)
#define ENABLE(WTF_FEATURE) (defined ENABLE_##WTF_FEATURE  && ENABLE_##WTF_FEATURE)
#define ASSERT(x)
#define ALWAYS_INLINE __inline__
#if defined(WIN32)
#define WTF_COMPILER_MSVC 1
#define WTF_CPU_BIG_ENDIAN 0
#define WTF_USE_EJSVALUE64 1
#define WTF_USE_EJSVALUE32_64 !WTF_USE_EJSVALUE64
#endif
#else
#define ENABLE_GC_VALIDATION 0
#endif

/**
 * TYPE PORTING PART
 */
class CodeGenContext;
typedef llvm::Value ExecState;
typedef llvm::Value RegisterID;
typedef std::string UString;
typedef enum
{
    op_enter,
    op_const_undefined,
    op_const_null,
    op_const_boolean,
    op_const_number,
    op_const_string,
    op_create_activation,
    op_init_lazy_reg,
    op_create_arguments,
    op_create_this,
    op_get_callee,
    op_convert_this,
    op_new_object,
    op_new_array,
    op_new_array_buffer,
    op_new_regexp,
    op_mov,
    op_not,
    op_and,
    op_or,
    op_plusplus,
    op_minusminus,
    op_pluseq,
    op_minuseq,
    op_multeq,
    op_diveq,
    op_andeq,
    op_oreq,
    op_xoreq,
    op_modeq,
    op_eq,
    op_eq_null,
    op_neq,
    op_neq_null,
    op_stricteq,
    op_nstricteq,
    op_less,
    op_lesseq,
    op_greater,
    op_greatereq,
    op_pre_inc,
    op_pre_dec,
    op_post_inc,
    op_post_dec,
    op_to_jsnumber,
    op_negate,
    op_add,
    op_mul,
    op_div,
    op_mod,
    op_sub,
    op_lshift,
    op_rshift,
    op_urshift,
    op_bitand,
    op_bitxor,
    op_bitor,
    op_bitnot,
    op_check_has_instance,
    op_instanceof,
    op_typeof,
    op_is_undefined,
    op_is_boolean,
    op_is_number,
    op_is_string,
    op_is_object,
    op_is_function,
    op_in,
    op_resolve,
    op_resolve_list,
    op_resolve_global,
    op_resolve_global_dynamic,
    op_get_scoped_var,
    op_put_scoped_var,
    op_get_global_var,
    op_put_global_var,
    op_resolve_base,
    op_ensure_property_exists,
    op_resolve_with_base,
    op_resolve_with_this,
    op_get_by_id,
    op_get_by_id_self,
    op_get_by_id_proto,
    op_get_by_id_chain,
    op_get_by_id_getter_self,
    op_get_by_id_getter_proto,
    op_get_by_id_getter_chain,
    op_get_by_id_custom_self,
    op_get_by_id_custom_proto,
    op_get_by_id_custom_chain,
    op_get_by_id_generic,
    op_get_array_length,
    op_get_string_length,
    op_get_arguments_length,
    op_put_by_id,
    op_put_by_id_transition,
    op_put_by_id_replace,
    op_put_by_id_generic,
    op_del_by_id,
    op_get_by_val,
    op_get_argument_by_val,
    op_get_by_pname,
    op_put_by_val,
    op_del_by_val,
    op_put_by_index,
    op_put_getter,
    op_put_setter,
    op_jmp,
    op_jtrue,
    op_jfalse,
    op_jeq_null,
    op_jneq_null,
    op_jneq_ptr,
    op_jless,
    op_jlesseq,
    op_jgreater,
    op_jgreatereq,
    op_jnless,
    op_jnlesseq,
    op_jngreater,
    op_jngreatereq,
    op_jmp_scopes,
    op_loop,
    op_loop_if_true,
    op_loop_if_false,
    op_loop_if_less,
    op_loop_if_lesseq,
    op_loop_if_greater,
    op_loop_if_greatereq,
    op_loop_hint,
    op_switch_imm,
    op_switch_char,
    op_switch_string,
    op_new_func,
    op_new_func_exp,
    op_call,
    op_call_eval,
    op_call_varargs,
    op_load_varargs,
    op_tear_off_activation,
    op_tear_off_arguments,
    op_ret,
    op_call_put_result,
    op_ret_object_or_this,
    op_method_check,
    op_construct,
    op_strcat,
    op_to_primitive,
    op_get_pnames,
    op_next_pname,
    op_push_scope,
    op_pop_scope,
    op_push_new_scope,
    op_catch,
    op_throw,
    op_throw_reference_error,
    op_to_number,
    op_jsr,
    op_sret,
    op_debug,
    op_profile_will_call,
    op_profile_did_call,
    op_end,
}OpcodeID;

class ClassInfo {
public:
    ClassInfo(const char *name = NULL, ClassInfo *parent = NULL)
        : className(name)
        , parentClass(parent)
    {
    }
    /**
     * A string denoting the class name. Example: "Window".
     */
    const char* className;

    /**
     * Pointer to the class information of the base class.
     * 0L if there is none.
     */
    const ClassInfo* parentClass;
    
    bool isSubClassOf(const ClassInfo* other) const
    {
        for (const ClassInfo* ci = this; ci; ci = ci->parentClass) {
            if (ci == other)
                return true;
        }
        return false;
    }
};

#endif /* end of #ifndef TYPEPORTINGFORJSC_H */
