
#include "util_mem.h"
#include "util_stream.h"
#include "io_proto.h"

//
// Decodes a protocol from a message, then encodes it again
//
_ext__ int32_t io_message_decode_encode(
    io_codec_id_t codec_id,
    int num_messages,
    uint8_t* in,
    int32_t in_len,
    uint8_t* out,
    int32_t out_len
)
{
    int32_t result;
    bool is_null;
    bool call_fini = false;
    io_codec_ctx_t ctx, obj;
    io_fixed_buffer_stream_t mem;
    io_stream_t* stream;
    io_message_factory_t* factory = NULL;
    io_message_t* message[10];
    memset(message, 0, sizeof(message));

    do
    {
        stream = io_fixed_buffer_stream_init(&mem, in, in_len, out, out_len);
        dbg_assert_ptr(stream);

        result = io_message_factory_create(2, 0, 0, NULL, NULL, &factory);
        if (result != er_ok)
            break;

        for (int i = 0; i < num_messages; i++)
        {
            result = io_codec_ctx_init(io_codec_by_id(codec_id), &ctx, stream, true, NULL);
            if (result != er_ok)
                break;
            call_fini = true;

            result = io_message_create_empty(factory, &message[i]);
            if (result != er_ok)
                break;

            // Decode message object
            result = io_decode_object(&ctx, NULL, &is_null, &obj);
            if (result != er_ok)
                break;
            result = io_decode_message(&obj, message[i]);
            if (result != er_ok)
                break;

            result = io_codec_ctx_fini(&ctx, stream, false);
            call_fini = false;
            if (result != er_ok)
                break;
        }

        result = io_stream_reset(stream);
        if (result != er_ok)
            break;

        // Reencode message object
        for (int i = 0; i < num_messages; i++)
        {
            result = io_codec_ctx_init(io_codec_by_id(codec_id), &ctx, stream, false, NULL);
            if (result != er_ok)
                break;
            call_fini = true;

            ctx.index = 1;
            result = io_encode_object(&ctx, NULL, is_null, &obj);
            if (result != er_ok)
                break;
            result = io_encode_message(&obj, message[i]);
            if (result != er_ok)
                break;

            result = io_codec_ctx_fini(&ctx, stream, true);
            call_fini = false;
            if (result != er_ok)
                break;
        }
    } 
    while (0);

    // Flush
    if (call_fini)
        (void)io_codec_ctx_fini(&ctx, stream, true);

    io_stream_close(stream);
    
    for (int i = 0; i < num_messages; i++)
    {
        if (message[i])
            io_message_release(message[i]);
    }

    if (factory)
        io_message_factory_free(factory);

    return result;
}