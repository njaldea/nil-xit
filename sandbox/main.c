#include <nil/service.h>
#include <nil/xit.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct UniqueData
{
    char* text;
} UniqueData;

uint64_t unique_encode_size(const void* ctx)
{
    return strlen(((const UniqueData*)ctx)->text);
}

// NOLINTNEXTLINE
void unique_encode(const void* ctx, void* buffer)
{
    const char* str = ((const UniqueData*)ctx)->text;
    memcpy(buffer, str, strlen(str));
}

// NOLINTNEXTLINE
void unique_decode(void* ctx, const void* data, uint64_t size)
{
    UniqueData* d = (UniqueData*)ctx;
    free(d->text);
    d->text = (char*)malloc(size);
    memcpy(d->text, data, size - 1U);
    d->text[size - 1U] = '\0';
}

void unique_cleanup(void* ctx)
{
    UniqueData* d = (UniqueData*)ctx;
    free(d->text);
    free(d);
}

typedef struct TaggedDataEntry
{
    char* tag;
    char* text;
} TaggedDataEntry;

typedef struct TaggedData
{
    size_t count;
    TaggedDataEntry** entries;
} TaggedData;

uint64_t tagged_encode_size(const char* tag, const void* ctx)
{
    for (size_t i = 0; i < ((const TaggedData*)ctx)->count; ++i)
    {
        if (strcmp(((const TaggedData*)ctx)->entries[i]->tag, tag) == 0)
        {
            return strlen(((const TaggedData*)ctx)->entries[i]->text);
        }
    }
    return 0;
}

// NOLINTNEXTLINE
void tagged_encode(const char* tag, const void* ctx, void* buffer)
{
    TaggedData* d = (TaggedData*)ctx;
    printf("Encoding tag: %s\n", tag);
    for (size_t i = 0; i < d->count; ++i)
    {
        if (strcmp(d->entries[i]->tag, tag) == 0)
        {
            memcpy(buffer, d->entries[i]->text, strlen(d->entries[i]->text));
            return;
        }
    }
}

// NOLINTNEXTLINE
void tagged_decode(const char* tag, void* ctx, const void* data, uint64_t size)
{
    TaggedData* d = (TaggedData*)ctx;
    for (size_t i = 0; i < d->count; ++i)
    {
        if (strcmp(d->entries[i]->tag, tag) == 0)
        {
            TaggedDataEntry* entry = d->entries[i];
            free(entry->text);
            entry->text = (char*)malloc(size);
            memcpy(entry->text, data, size - 1);
            entry->text[size - 1U] = '\0';
            return;
        }
    }
}

void tagged_cleanup(void* ctx)
{
    TaggedData* d = (TaggedData*)ctx;
    for (size_t i = 0; i < d->count; ++i)
    {
        free(d->entries[i]->tag);
        free(d->entries[i]->text);
        free(d->entries[i]);
    }
    free(d->entries);
    free(d);
}

struct cin_context
{
    nil_xit_unique_frame_value* unique_value;
    nil_xit_tagged_frame_value* tagged_value;
    int index;
};

void* post_value_thread(void* ctx)
{
    struct cin_context* context = (struct cin_context*)ctx;
    char line[1024];
    while (fgets(line, sizeof(line), stdin))
    {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }
        switch(context->index)
        {
            case 0: {
                char json[1200];
                snprintf(json, sizeof(json), "{ \"message\": \"%s\" }", line);
                nil_xit_unique_value_post(*context->unique_value, &json, strlen(json));
                context->index = 1;
                break;
            }
            case 1: {
                char json[1200];
                snprintf(json, sizeof(json), "{ \"message\": \"%s\" }", line);
                nil_xit_tagged_value_post(*context->tagged_value, "1101", &json, strlen(json));
                context->index = 2;
                break;
            }
            case 2: {
                char json[1200];
                snprintf(json, sizeof(json), "{ \"message\": \"%s\" }", line);
                nil_xit_tagged_value_post(*context->tagged_value, "1102", &json, strlen(json));
                context->index = 0;
                break;
            }
        }
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
    const char* assets[] = { "assets", "assets/xit/assets" };
    nil_xit_setup_server(web, assets, 2);

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

    nil_xit_core_add_unique_frame(
        core,
        "index",
        &(nil_xit_file_info){.group = "base", .path = "gui/Demo.svelte"}
    );

    nil_xit_unique_frame unique_frame = nil_xit_core_add_unique_frame(
        core,
        "json_editor",
        &(nil_xit_file_info){.group = "base", .path = "gui/JsonEditor.svelte"}
    );

    static const char initial[] = "{ \"message\": \"hello from c\" }";
    char* buf = (char*)malloc(sizeof(initial) - 1U);
    memcpy((void*)buf, initial, sizeof(initial) - 1U);

    UniqueData* unique_data = (UniqueData*)malloc(sizeof(UniqueData));
    unique_data->text = buf;

    nil_xit_unique_frame_value unique_value = nil_xit_unique_frame_add_value(
        unique_frame,
        "json_value",
        (nil_xit_unique_value_accessor
        ){.encode_size = &unique_encode_size,
          .encode = &unique_encode,
          .decode = &unique_decode,
          .ctx = unique_data,
          .cleanup = &unique_cleanup}
    );

    nil_service_web_on_ready(
        web,
        (nil_service_callback_info){.exec = on_ready, .context = NULL, .cleanup = NULL}
    );

    nil_xit_tagged_frame tagged_frame = nil_xit_core_add_tagged_frame(
        core,
        "tagged",
        &(nil_xit_file_info){.group = "base", .path = "gui/Tagged.svelte"}
    );

    TaggedDataEntry* entry1 = (TaggedDataEntry*)malloc(sizeof(TaggedDataEntry));
    entry1->tag = strdup("1101");
    entry1->text = strdup("{ \"message\": \"hello from c tagged first\" }");

    TaggedDataEntry* entry2 = (TaggedDataEntry*)malloc(sizeof(TaggedDataEntry));
    entry2->tag = strdup("1102");
    entry2->text = strdup("{ \"message\": \"hello from c tagged second\" }");

    TaggedData* tagged_data = (TaggedData*)malloc(sizeof(TaggedData));
    tagged_data->count = 2;
    tagged_data->entries = (TaggedDataEntry**)malloc(sizeof(TaggedDataEntry*) * tagged_data->count);
    tagged_data->entries[0] = entry1;
    tagged_data->entries[1] = entry2;

    nil_xit_tagged_frame_value tagged_value = nil_xit_tagged_frame_add_value(
        tagged_frame,
        "tagged_json",
        (nil_xit_tagged_value_accessor
        ){.encode_size = &tagged_encode_size,
          .encode = &tagged_encode,
          .decode = &tagged_decode,
          .ctx = tagged_data,
          .cleanup = &tagged_cleanup}
    );

    struct cin_context context = {
        .unique_value = &unique_value,
        .tagged_value = &tagged_value,
        .index = 0,
    };

    pthread_t tid = 0;
    pthread_create(&tid, NULL, post_value_thread, &context);
    pthread_detach(tid);

    nil_service_runnable_run(run);
    nil_xit_core_destroy(core);
    nil_service_web_destroy(web);
    return 0;
}
