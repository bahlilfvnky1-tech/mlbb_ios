#pragma once
#include <stdint.h>
#include <stddef.h>

extern "C" {
    extern const void *(*il2cpp_assembly_get_image)(const void *assembly);
    extern void *(*il2cpp_domain_get)();
    extern void **(*il2cpp_domain_get_assemblies)(const void *domain, size_t *size);
    extern const char *(*il2cpp_image_get_name)(void *image);
    extern void *(*il2cpp_class_from_name)(const void *image, const char *namespaze, const char *name);
    extern void *(*il2cpp_class_get_field_from_name)(void *klass, const char *name);
    extern void *(*il2cpp_class_get_method_from_name)(void *klass, const char *name, int argsCount);
    extern size_t (*il2cpp_field_get_offset)(void *field);
    extern void (*il2cpp_field_static_get_value)(void *field, void *value);
    extern void (*il2cpp_field_static_set_value)(void *field, void *value);
}

bool Il2CppAttach();
void *Il2CppGetImageByName(const char *image);
void *Il2CppGetClassType(const char *image, const char *namespaze, const char *clazz);
void Il2CppGetStaticFieldValue(const char *image, const char *namespaze, const char *clazz, const char *name, void *output);
size_t Il2CppGetFieldOffset(const char *image, const char *namespaze, const char *clazz, const char *name);
