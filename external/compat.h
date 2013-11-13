#ifndef _RCC_EXTERNAL_COMPAT_H
#define _RCC_EXTERNAL_COMPAT_H

# if GLIB_CHECK_VERSION(2,32,0)
inline static GMutex *g_mutex_new_32() {
    GMutex *res = malloc(sizeof(GMutex));
    if (!res) return res;
    g_mutex_init(res);
    return res;
}

inline static GCond *g_cond_new_32() {
    GCond *res = malloc(sizeof(GCond));
    if (!res) return res;
    g_cond_init(res);
    return res;
}

inline static GThread *g_thread_create_32(GThreadFunc func, gpointer data, int joinable) {
    GThread *res = g_thread_new(NULL, func, data);
    if ((res)&&(!joinable)) g_thread_unref(res);
    return res;
}

# define g_thread_create_compat(func, data, joinable) g_thread_create_32(func, data, joinable)
# define g_mutex_new_compat() g_mutex_new_32()
# define g_cond_new_compat() g_cond_new_32()
# define g_mutex_free_compat(mutex) free(mutex)
# define g_cond_free_compat(mutex) free(mutex)
#else
# define g_thread_create_compat(func, data, joinable) g_thread_create(func, data, joinable, NULL)
# define g_mutex_new_compat() g_mutex_new()
# define g_cond_new_compat() g_cond_new()
# define g_mutex_free_compat(mutex) g_mutex_free(mutex)
# define g_cond_free_compat(mutex) g_cond_free(mutex)
# endif 



#endif /*  _RCC_EXTERNAL_COMPAT_H */
