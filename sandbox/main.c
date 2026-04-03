#include <nil/service.h>
#include <nil/xit.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void on_load(void* ctx)
{
    (void)ctx;
    printf("frame loaded\n");
}

static void on_ready(const nil_service_id* id, void* ctx)
{
    (void)id;
    (void)ctx;
    printf("ready: http://127.0.0.1:1101/?frame=json_editor\n");
}

static void get_source_dir(char* out, size_t out_size)
{
    const char* file_path = __FILE__;
    size_t len = strlen(file_path);
    if (len >= out_size)
    {
        len = out_size - 1U;
    }

    memcpy(out, file_path, len);
    out[len] = '\0';

    for (size_t i = len; i > 0U; --i)
    {
        if (out[i - 1U] == '/')
        {
            out[i - 1U] = '\0';
            return;
        }
    }
}

typedef struct Data
{
    char* text;
} Data;

uint64_t encode_size(const void* ctx)
{
    return strlen(((const Data*)ctx)->text);
}

// NOLINTNEXTLINE
void encode(const void* ctx, void* buffer)
{
    const char* str = ((const Data*)ctx)->text;
    memcpy(buffer, str, strlen(str));
}

// NOLINTNEXTLINE
void decode(void* ctx, const void* data, uint64_t size)
{
    Data* d = (Data*)ctx;
    free(d->text);
    d->text = (char*)malloc(size + 1U);
    memcpy(d->text, data, size);
    d->text[size] = '\0';
}

void cleanup(void* ctx)
{
    Data* d = (Data*)ctx;
    free(d->text);
    free(d);
}

void* post_value_thread(void* ctx)
{
    nil_xit_unique_frame_value* value = (nil_xit_unique_frame_value*)ctx;
    char line[1024];
    while (fgets(line, sizeof(line), stdin))
    {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }
        char json[1200];
        snprintf(json, sizeof(json), "{ \"message\": \"%s\" }", line);
        Data data = {.text = json};
        nil_xit_unique_value_post(*value, &data);
    }
    return NULL;
}

int main(void)
{
    char source_path[1024];
    char components_path[1200];
    get_source_dir(source_path, sizeof(source_path));
    (void)snprintf(components_path, sizeof(components_path), "%s/gui/components", source_path);

    nil_service_web web = nil_service_create_http_server("127.0.0.1", 1101, 4096);
    nil_service_event ws = nil_service_web_use_ws(web, "/ws");
    nil_service_runnable run = nil_service_web_to_runnable(web);
    nil_xit_setup_server(web, "assets/xit/assets");

    nil_xit_core core = nil_xit_core_create(run, ws);
    nil_xit_set_cache_directory(core, "/tmp/nil-xit-sandbox-c");
    nil_xit_set_groups(
        core,
        (nil_xit_group_entry[]){
            {.group = "base", .path = source_path},
            {.group = "components", .path = components_path},
        },
        2
    );

    nil_xit_unique_frame frame
        = nil_xit_core_add_unique_frame(core, "json_editor", "$base/gui/JsonEditor.svelte");

    static const char initial[] = "{ \"message\": \"hello from c\" }";
    char* buf = (char*)malloc(sizeof(initial) - 1U);
    memcpy((void*)buf, initial, sizeof(initial) - 1U);

    Data* data = (Data*)malloc(sizeof(Data));
    data->text = buf;

    nil_xit_unique_frame_value value = nil_xit_unique_frame_add_value(
        frame,
        "json_value",
        (nil_xit_unique_value_accessor
        ){.encode_size = &encode_size,
          .encode = &encode,
          .decode = &decode,
          .ctx = data,
          .cleanup = &cleanup}
    );

    nil_xit_unique_frame_on_load(
        frame,
        (nil_xit_callback_info){.exec = on_load, .context = NULL, .cleanup = NULL}
    );

    nil_service_web_on_ready(
        web,
        (nil_service_callback_info){.exec = on_ready, .context = NULL, .cleanup = NULL}
    );

    pthread_t tid = 0;
    pthread_create(&tid, NULL, post_value_thread, &value);
    pthread_detach(tid);

    nil_service_runnable_start(run);
    nil_xit_core_destroy(core);
    nil_service_web_destroy(web);
    return 0;
}
