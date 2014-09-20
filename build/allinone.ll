; ModuleID = '..\ejs_runtime\allinone_for_load.c'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

%union.ejsval = type { i64 }

@.str = private unnamed_addr constant [14 x i8] c"litterendian\0A\00", align 1
@jsValue32PayloadPart = external global i32
@jsValue32TagPart = external global i32
@isLitterEndian = external global i32
@.str1 = private unnamed_addr constant [11 x i8] c"bigendian\0A\00", align 1
@_ejs_nan = external global %union.ejsval
@jsPositiveInfinity = external global %union.ejsval
@jsNegativeInfinity = external global %union.ejsval
@_ejs_null = external global %union.ejsval
@_ejs_undefined = external global %union.ejsval
@_ejs_true = external global %union.ejsval
@_ejs_false = external global %union.ejsval
@_ejs_one = external global %union.ejsval
@_ejs_zero = external global %union.ejsval
@jsMin = external global %union.ejsval
@jsMax = external global %union.ejsval
@.str2 = private unnamed_addr constant [7 x i8] c"%s:%d\0A\00", align 1
@__FUNCTION__.AllInOneInit = private unnamed_addr constant [13 x i8] c"AllInOneInit\00", align 1
@_ejs_global = external global %union.ejsval
@_ejs_console = external global %union.ejsval
@_ejs_Object = external global %union.ejsval
@_ejs_Boolean = external global %union.ejsval
@_ejs_Number = external global %union.ejsval
@_ejs_String = external global %union.ejsval
@_ejs_Array = external global %union.ejsval
@_ejs_Function = external global %union.ejsval
@_ejs_Process = external global %union.ejsval
@_ejs_Symbol_create = external global %union.ejsval
@_ejs_Math = external global %union.ejsval
@_ejs_JSON = external global %union.ejsval
@jsValue32Size = external global i32

; Function Attrs: nounwind
define i32 @IsLitterEndian() #0 {
entry:
  %retval = alloca i32, align 4
  %u = alloca %union.ejsval, align 8
  %asInt64 = bitcast %union.ejsval* %u to i64*
  store i64 1, i64* %asInt64, align 8
  %asIntPtr = bitcast %union.ejsval* %u to [2 x i32]*
  %arrayidx = getelementptr inbounds [2 x i32]* %asIntPtr, i32 0, i32 0
  %0 = load i32* %arrayidx, align 4
  %cmp = icmp eq i32 1, %0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([14 x i8]* @.str, i32 0, i32 0))
  store i32 0, i32* @jsValue32PayloadPart, align 4
  store i32 1, i32* @jsValue32TagPart, align 4
  store i32 1, i32* @isLitterEndian, align 4
  store i32 1, i32* %retval
  br label %return

if.end:                                           ; preds = %entry
  %call1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([11 x i8]* @.str1, i32 0, i32 0))
  store i32 0, i32* %retval
  br label %return

return:                                           ; preds = %if.end, %if.then
  %1 = load i32* %retval
  ret i32 %1
}

declare i32 @printf(i8*, ...) #1

; Function Attrs: nounwind
define void @AllInOneInit() #0 {
entry:
  %call = call i32 @IsLitterEndian()
  store i64 9221120237041090560, i64* getelementptr inbounds (%union.ejsval* @_ejs_nan, i32 0, i32 0), align 8
  store i64 9218868437227405312, i64* getelementptr inbounds (%union.ejsval* @jsPositiveInfinity, i32 0, i32 0), align 8
  store i64 -4503599627370496, i64* getelementptr inbounds (%union.ejsval* @jsNegativeInfinity, i32 0, i32 0), align 8
  %0 = load i32* @jsValue32TagPart, align 4
  %arrayidx = getelementptr inbounds [2 x i32]* bitcast (%union.ejsval* @_ejs_null to [2 x i32]*), i32 0, i32 %0
  store i32 -121, i32* %arrayidx, align 4
  %1 = load i32* @jsValue32PayloadPart, align 4
  %arrayidx1 = getelementptr inbounds [2 x i32]* bitcast (%union.ejsval* @_ejs_null to [2 x i32]*), i32 0, i32 %1
  store i32 0, i32* %arrayidx1, align 4
  %2 = load i32* @jsValue32TagPart, align 4
  %arrayidx2 = getelementptr inbounds [2 x i32]* bitcast (%union.ejsval* @_ejs_undefined to [2 x i32]*), i32 0, i32 %2
  store i32 -126, i32* %arrayidx2, align 4
  %3 = load i32* @jsValue32PayloadPart, align 4
  %arrayidx3 = getelementptr inbounds [2 x i32]* bitcast (%union.ejsval* @_ejs_undefined to [2 x i32]*), i32 0, i32 %3
  store i32 0, i32* %arrayidx3, align 4
  %4 = load i32* @jsValue32TagPart, align 4
  %arrayidx4 = getelementptr inbounds [2 x i32]* bitcast (%union.ejsval* @_ejs_true to [2 x i32]*), i32 0, i32 %4
  store i32 -123, i32* %arrayidx4, align 4
  %5 = load i32* @jsValue32PayloadPart, align 4
  %arrayidx5 = getelementptr inbounds [2 x i32]* bitcast (%union.ejsval* @_ejs_true to [2 x i32]*), i32 0, i32 %5
  store i32 1, i32* %arrayidx5, align 4
  %6 = load i32* @jsValue32TagPart, align 4
  %arrayidx6 = getelementptr inbounds [2 x i32]* bitcast (%union.ejsval* @_ejs_false to [2 x i32]*), i32 0, i32 %6
  store i32 -123, i32* %arrayidx6, align 4
  %7 = load i32* @jsValue32PayloadPart, align 4
  %arrayidx7 = getelementptr inbounds [2 x i32]* bitcast (%union.ejsval* @_ejs_false to [2 x i32]*), i32 0, i32 %7
  store i32 0, i32* %arrayidx7, align 4
  store double 1.000000e+00, double* bitcast (%union.ejsval* @_ejs_one to double*), align 8
  store double 0.000000e+00, double* bitcast (%union.ejsval* @_ejs_zero to double*), align 8
  store double 4.940656e-324, double* bitcast (%union.ejsval* @jsMin to double*), align 8
  store double 0x7FEFFFFFFFFFFFFF, double* bitcast (%union.ejsval* @jsMax to double*), align 8
  call void @_ejs_init(i32 0, i8** null)
  %call8 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([7 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([13 x i8]* @__FUNCTION__.AllInOneInit, i32 0, i32 0), i32 62)
  ret void
}

declare void @_ejs_init(i32, i8**) #1

; Function Attrs: nounwind
define void @allinone_for_load_just_ensure_these_functions_and_variables_are_included_please_do_not_call() #0 {
entry:
  %coerce = alloca %union.ejsval, align 8
  %coerce34 = alloca %union.ejsval, align 8
  %coerce37 = alloca %union.ejsval, align 8
  %coerce43 = alloca %union.ejsval, align 8
  %coerce46 = alloca %union.ejsval, align 8
  %coerce49 = alloca %union.ejsval, align 8
  %coerce56 = alloca %union.ejsval, align 8
  %coerce59 = alloca %union.ejsval, align 8
  %coerce63 = alloca %union.ejsval, align 8
  %coerce66 = alloca %union.ejsval, align 8
  %coerce69 = alloca %union.ejsval, align 8
  %coerce72 = alloca %union.ejsval, align 8
  %coerce75 = alloca %union.ejsval, align 8
  %coerce78 = alloca %union.ejsval, align 8
  %coerce81 = alloca %union.ejsval, align 8
  %coerce84 = alloca %union.ejsval, align 8
  %coerce87 = alloca %union.ejsval, align 8
  %coerce90 = alloca %union.ejsval, align 8
  %coerce93 = alloca %union.ejsval, align 8
  %coerce96 = alloca %union.ejsval, align 8
  %coerce99 = alloca %union.ejsval, align 8
  %coerce102 = alloca %union.ejsval, align 8
  %coerce113 = alloca %union.ejsval, align 8
  %coerce116 = alloca %union.ejsval, align 8
  %coerce119 = alloca %union.ejsval, align 8
  %coerce122 = alloca %union.ejsval, align 8
  %coerce125 = alloca %union.ejsval, align 8
  %coerce128 = alloca %union.ejsval, align 8
  %call = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_nan)
  %call1 = call i32 @JSValueHash(%union.ejsval* byval align 4 @jsPositiveInfinity)
  %call2 = call i32 @JSValueHash(%union.ejsval* byval align 4 @jsNegativeInfinity)
  %call3 = call i32 @JSValueHash(%union.ejsval* byval align 4 @jsMax)
  %call4 = call i32 @JSValueHash(%union.ejsval* byval align 4 @jsMin)
  %call5 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_null)
  %call6 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_undefined)
  %call7 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_true)
  %call8 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_false)
  %call9 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_one)
  %call10 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_zero)
  %call11 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_global)
  %call12 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_console)
  %call13 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_Object)
  %call14 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_Boolean)
  %call15 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_Number)
  %call16 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_String)
  %call17 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_Array)
  %call18 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_Function)
  %call19 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_Process)
  %call20 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_Symbol_create)
  %call21 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_Math)
  %call22 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_JSON)
  call void bitcast (void (...)* @jsextern_print_tick to void ()*)()
  call void @jsextern_os_msleep(i32 0)
  %call23 = call zeroext i1 @jsextern_os_swap(i64* null, i64 0, i64 0)
  %call24 = call i8* @jsextern_pcre_compile(i8* null)
  %call25 = call i8* @jsextern_pcre_study(i8* null)
  %call26 = call i32 @jsextern_pcre_bracketcount(i8* null)
  %call27 = call i32 @jsextern_pcre_exec(i8* null, i8* null, i8* null, i32 0, i32 0, i32* null, i32 0)
  call void @jsextern_pcre_free(i8* null)
  %call28 = call i32 @jsextern_thread_create(i8* (i8*)* null, i8* null)
  call void @jsextern_thread_destroy(i32 0)
  %call29 = call i32 bitcast (i32 (...)* @jsextern_mutex_create to i32 ()*)()
  call void @jsextern_mutex_destroy(i32 0)
  call void @jsextern_mutex_lock(i32 0)
  call void @jsextern_mutex_unlock(i32 0)
  %call30 = call i32 bitcast (i32 (...)* @jsextern_signal_create to i32 ()*)()
  call void @jsextern_signal_destroy(i32 0)
  call void @jsextern_signal_wait(i32 0)
  call void @jsextern_signal_send(i32 0)
  %call31 = call i32 @JSValueHash(%union.ejsval* byval align 4 @_ejs_undefined)
  %call32 = call i64 @_ejs_eval(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined, i32 0, %union.ejsval* null)
  %coerce.dive = getelementptr %union.ejsval* %coerce, i32 0, i32 0
  store i64 %call32, i64* %coerce.dive
  %call33 = call i64 @_ejs_object_getprop_utf8(%union.ejsval* byval align 4 @_ejs_undefined, i8* null)
  %coerce.dive35 = getelementptr %union.ejsval* %coerce34, i32 0, i32 0
  store i64 %call33, i64* %coerce.dive35
  %call36 = call i64 @_ejs_object_setprop_utf8(%union.ejsval* byval align 4 @_ejs_global, i8* null, %union.ejsval* byval align 4 @_ejs_undefined)
  %coerce.dive38 = getelementptr %union.ejsval* %coerce37, i32 0, i32 0
  store i64 %call36, i64* %coerce.dive38
  %call39 = call zeroext i1 @_ejs_object_define_value_property(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined, i32 0)
  %call40 = call zeroext i1 @_ejs_object_define_getter_property(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined, i32 0)
  %call41 = call zeroext i1 @_ejs_object_define_setter_property(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined, i32 0)
  %call42 = call i64 @_ejs_Object_create(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined, i32 0, %union.ejsval* null)
  %coerce.dive44 = getelementptr %union.ejsval* %coerce43, i32 0, i32 0
  store i64 %call42, i64* %coerce.dive44
  %call45 = call i64 @_ejs_Object_getOwnPropertyNames(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined, i32 0, %union.ejsval* null)
  %coerce.dive47 = getelementptr %union.ejsval* %coerce46, i32 0, i32 0
  store i64 %call45, i64* %coerce.dive47
  %call48 = call i64 @_ejs_specop_get(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %coerce.dive50 = getelementptr %union.ejsval* %coerce49, i32 0, i32 0
  store i64 %call48, i64* %coerce.dive50
  %call51 = call zeroext i1 @_ejs_specop_set(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %call52 = call zeroext i1 @ToEJSBool(%union.ejsval* byval align 4 @_ejs_undefined)
  %call53 = call double @ToDouble(%union.ejsval* byval align 4 @_ejs_undefined)
  %call54 = call i32 @ToUint32(%union.ejsval* byval align 4 @_ejs_undefined)
  %call55 = call i64 @_ejs_op_typeof(%union.ejsval* byval align 4 @_ejs_undefined)
  %coerce.dive57 = getelementptr %union.ejsval* %coerce56, i32 0, i32 0
  store i64 %call55, i64* %coerce.dive57
  %call58 = call i64 @_ejs_op_instanceof(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %coerce.dive60 = getelementptr %union.ejsval* %coerce59, i32 0, i32 0
  store i64 %call58, i64* %coerce.dive60
  %call61 = call zeroext i1 @_ejs_op_typeof_is_array(%union.ejsval* byval align 4 @_ejs_undefined)
  %call62 = call i64 @_ejs_op_plusplus(%union.ejsval* byval align 4 @_ejs_undefined, i1 zeroext false)
  %coerce.dive64 = getelementptr %union.ejsval* %coerce63, i32 0, i32 0
  store i64 %call62, i64* %coerce.dive64
  %call65 = call i64 @_ejs_op_minusminus(%union.ejsval* byval align 4 @_ejs_undefined, i1 zeroext false)
  %coerce.dive67 = getelementptr %union.ejsval* %coerce66, i32 0, i32 0
  store i64 %call65, i64* %coerce.dive67
  %call68 = call i64 @_ejs_op_bitwise_xor(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %coerce.dive70 = getelementptr %union.ejsval* %coerce69, i32 0, i32 0
  store i64 %call68, i64* %coerce.dive70
  %call71 = call i64 @_ejs_op_bitwise_and(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %coerce.dive73 = getelementptr %union.ejsval* %coerce72, i32 0, i32 0
  store i64 %call71, i64* %coerce.dive73
  %call74 = call i64 @_ejs_op_bitwise_or(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %coerce.dive76 = getelementptr %union.ejsval* %coerce75, i32 0, i32 0
  store i64 %call74, i64* %coerce.dive76
  %call77 = call i64 @_ejs_op_rsh(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %coerce.dive79 = getelementptr %union.ejsval* %coerce78, i32 0, i32 0
  store i64 %call77, i64* %coerce.dive79
  %call80 = call i64 @_ejs_op_ursh(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %coerce.dive82 = getelementptr %union.ejsval* %coerce81, i32 0, i32 0
  store i64 %call80, i64* %coerce.dive82
  %call83 = call i64 @_ejs_op_lsh(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %coerce.dive85 = getelementptr %union.ejsval* %coerce84, i32 0, i32 0
  store i64 %call83, i64* %coerce.dive85
  %call86 = call i64 @_ejs_op_ulsh(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %coerce.dive88 = getelementptr %union.ejsval* %coerce87, i32 0, i32 0
  store i64 %call86, i64* %coerce.dive88
  %call89 = call i64 @_ejs_op_mod(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %coerce.dive91 = getelementptr %union.ejsval* %coerce90, i32 0, i32 0
  store i64 %call89, i64* %coerce.dive91
  %call92 = call i64 @_ejs_op_add(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %coerce.dive94 = getelementptr %union.ejsval* %coerce93, i32 0, i32 0
  store i64 %call92, i64* %coerce.dive94
  %call95 = call i64 @_ejs_op_sub(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %coerce.dive97 = getelementptr %union.ejsval* %coerce96, i32 0, i32 0
  store i64 %call95, i64* %coerce.dive97
  %call98 = call i64 @_ejs_op_mult(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %coerce.dive100 = getelementptr %union.ejsval* %coerce99, i32 0, i32 0
  store i64 %call98, i64* %coerce.dive100
  %call101 = call i64 @_ejs_op_div(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %coerce.dive103 = getelementptr %union.ejsval* %coerce102, i32 0, i32 0
  store i64 %call101, i64* %coerce.dive103
  %call104 = call zeroext i1 @_ejs_op_lt(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %call105 = call zeroext i1 @_ejs_op_le(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %call106 = call zeroext i1 @_ejs_op_gt(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %call107 = call zeroext i1 @_ejs_op_ge(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %call108 = call zeroext i1 @_ejs_op_strict_eq(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %call109 = call zeroext i1 @_ejs_op_strict_neq(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %call110 = call zeroext i1 @_ejs_op_eq(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %call111 = call zeroext i1 @_ejs_op_neq(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined)
  %call112 = call i64 @_ejs_arguments_new(i32 0, %union.ejsval* null)
  %coerce.dive114 = getelementptr %union.ejsval* %coerce113, i32 0, i32 0
  store i64 %call112, i64* %coerce.dive114
  %0 = load i32* @jsValue32Size, align 4
  %call115 = call i64 @_ejs_array_new(i32 %0, i1 zeroext false)
  %coerce.dive117 = getelementptr %union.ejsval* %coerce116, i32 0, i32 0
  store i64 %call115, i64* %coerce.dive117
  %call118 = call i64 @_ejs_string_new_utf8_len(i8* null, i32 0)
  %coerce.dive120 = getelementptr %union.ejsval* %coerce119, i32 0, i32 0
  store i64 %call118, i64* %coerce.dive120
  %call121 = call i64 @_ejs_function_new_utf8(%union.ejsval* byval align 4 @_ejs_undefined, i8* null, i64 (%union.ejsval*, %union.ejsval*, i32, %union.ejsval*)* null)
  %coerce.dive123 = getelementptr %union.ejsval* %coerce122, i32 0, i32 0
  store i64 %call121, i64* %coerce.dive123
  %call124 = call i64 @_ejs_invoke_closure(%union.ejsval* byval align 4 @_ejs_undefined, %union.ejsval* byval align 4 @_ejs_undefined, i32 0, %union.ejsval* null)
  %coerce.dive126 = getelementptr %union.ejsval* %coerce125, i32 0, i32 0
  store i64 %call124, i64* %coerce.dive126
  %call127 = call i64 @_ejs_regexp_new_utf8(i8* null, i8* null)
  %coerce.dive129 = getelementptr %union.ejsval* %coerce128, i32 0, i32 0
  store i64 %call127, i64* %coerce.dive129
  ret void
}

declare i32 @JSValueHash(%union.ejsval* byval align 4) #1

declare void @jsextern_print_tick(...) #1

declare void @jsextern_os_msleep(i32) #1

declare zeroext i1 @jsextern_os_swap(i64*, i64, i64) #1

declare i8* @jsextern_pcre_compile(i8*) #1

declare i8* @jsextern_pcre_study(i8*) #1

declare i32 @jsextern_pcre_bracketcount(i8*) #1

declare i32 @jsextern_pcre_exec(i8*, i8*, i8*, i32, i32, i32*, i32) #1

declare void @jsextern_pcre_free(i8*) #1

declare i32 @jsextern_thread_create(i8* (i8*)*, i8*) #1

declare void @jsextern_thread_destroy(i32) #1

declare i32 @jsextern_mutex_create(...) #1

declare void @jsextern_mutex_destroy(i32) #1

declare void @jsextern_mutex_lock(i32) #1

declare void @jsextern_mutex_unlock(i32) #1

declare i32 @jsextern_signal_create(...) #1

declare void @jsextern_signal_destroy(i32) #1

declare void @jsextern_signal_wait(i32) #1

declare void @jsextern_signal_send(i32) #1

declare i64 @_ejs_eval(%union.ejsval* byval align 4, %union.ejsval* byval align 4, i32, %union.ejsval*) #1

declare i64 @_ejs_object_getprop_utf8(%union.ejsval* byval align 4, i8*) #1

declare i64 @_ejs_object_setprop_utf8(%union.ejsval* byval align 4, i8*, %union.ejsval* byval align 4) #1

declare zeroext i1 @_ejs_object_define_value_property(%union.ejsval* byval align 4, %union.ejsval* byval align 4, %union.ejsval* byval align 4, i32) #1

declare zeroext i1 @_ejs_object_define_getter_property(%union.ejsval* byval align 4, %union.ejsval* byval align 4, %union.ejsval* byval align 4, i32) #1

declare zeroext i1 @_ejs_object_define_setter_property(%union.ejsval* byval align 4, %union.ejsval* byval align 4, %union.ejsval* byval align 4, i32) #1

declare i64 @_ejs_Object_create(%union.ejsval* byval align 4, %union.ejsval* byval align 4, i32, %union.ejsval*) #1

declare i64 @_ejs_Object_getOwnPropertyNames(%union.ejsval* byval align 4, %union.ejsval* byval align 4, i32, %union.ejsval*) #1

declare i64 @_ejs_specop_get(%union.ejsval* byval align 4, %union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare zeroext i1 @_ejs_specop_set(%union.ejsval* byval align 4, %union.ejsval* byval align 4, %union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare zeroext i1 @ToEJSBool(%union.ejsval* byval align 4) #1

declare double @ToDouble(%union.ejsval* byval align 4) #1

declare i32 @ToUint32(%union.ejsval* byval align 4) #1

declare i64 @_ejs_op_typeof(%union.ejsval* byval align 4) #1

declare i64 @_ejs_op_instanceof(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare zeroext i1 @_ejs_op_typeof_is_array(%union.ejsval* byval align 4) #1

declare i64 @_ejs_op_plusplus(%union.ejsval* byval align 4, i1 zeroext) #1

declare i64 @_ejs_op_minusminus(%union.ejsval* byval align 4, i1 zeroext) #1

declare i64 @_ejs_op_bitwise_xor(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare i64 @_ejs_op_bitwise_and(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare i64 @_ejs_op_bitwise_or(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare i64 @_ejs_op_rsh(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare i64 @_ejs_op_ursh(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare i64 @_ejs_op_lsh(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare i64 @_ejs_op_ulsh(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare i64 @_ejs_op_mod(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare i64 @_ejs_op_add(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare i64 @_ejs_op_sub(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare i64 @_ejs_op_mult(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare i64 @_ejs_op_div(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare zeroext i1 @_ejs_op_lt(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare zeroext i1 @_ejs_op_le(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare zeroext i1 @_ejs_op_gt(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare zeroext i1 @_ejs_op_ge(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare zeroext i1 @_ejs_op_strict_eq(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare zeroext i1 @_ejs_op_strict_neq(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare zeroext i1 @_ejs_op_eq(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare zeroext i1 @_ejs_op_neq(%union.ejsval* byval align 4, %union.ejsval* byval align 4) #1

declare i64 @_ejs_arguments_new(i32, %union.ejsval*) #1

declare i64 @_ejs_array_new(i32, i1 zeroext) #1

declare i64 @_ejs_string_new_utf8_len(i8*, i32) #1

declare i64 @_ejs_function_new_utf8(%union.ejsval* byval align 4, i8*, i64 (%union.ejsval*, %union.ejsval*, i32, %union.ejsval*)*) #1

declare i64 @_ejs_invoke_closure(%union.ejsval* byval align 4, %union.ejsval* byval align 4, i32, %union.ejsval*) #1

declare i64 @_ejs_regexp_new_utf8(i8*, i8*) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!3}

!0 = metadata !{i32 6, metadata !"Linker Options", metadata !1}
!1 = metadata !{metadata !2}
!2 = metadata !{metadata !"/DEFAULTLIB:libcpmt.lib"}
!3 = metadata !{metadata !"clang version 3.4.2 (tags/RELEASE_34/dot2-final)"}
