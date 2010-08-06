#include "common.h"
#include "gif_encoder.h"
#include "gif.h"

using namespace v8;
using namespace node;

void
Gif::Initialize(Handle<Object> target)
{
    HandleScope scope;

    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(t, "encode", GifEncode);
    NODE_SET_PROTOTYPE_METHOD(t, "setTransparencyColor", SetTransparencyColor);
    target->Set(String::NewSymbol("Gif"), t->GetFunction());
}

Gif::Gif(Buffer *ddata, int wwidth, int hheight, buffer_type bbuf_type) :
    data(ddata), width(wwidth), height(hheight), buf_type(bbuf_type) {}

Handle<Value>
Gif::GifEncode()
{
    HandleScope scope;

    try {
        GifEncoder encoder((unsigned char *)data->data(), width, height, buf_type);
        if (transparency_color.color_present) {
            encoder.set_transparency_color(transparency_color);
        }
        encoder.encode();
        return scope.Close(
            Encode((char *)encoder.get_gif(), encoder.get_gif_len(), BINARY)
        );
    }
    catch (const char *err) {
        return VException(err);
    }
}

void
Gif::SetTransparencyColor(unsigned char r, unsigned char g, unsigned char b)
{
    transparency_color = Color(r, g, b, true);
}

Handle<Value>
Gif::New(const Arguments &args)
{
    HandleScope scope;

    if (args.Length() < 3)
        return VException("At least three arguments required - data buffer, width, height, [and input buffer type]");
    if (!Buffer::HasInstance(args[0]))
        return VException("First argument must be Buffer.");
    if (!args[1]->IsInt32())
        return VException("Second argument must be integer width.");
    if (!args[2]->IsInt32())
        return VException("Third argument must be integer height.");

    buffer_type buf_type = BUF_RGB;
    if (args.Length() == 4) {
        if (!args[3]->IsString())
            return VException("Fourth argument must be 'rgb', 'bgr', 'rgba' or 'bgra'.");

        String::AsciiValue bts(args[3]->ToString());
        if (!(str_eq(*bts, "rgb") || str_eq(*bts, "bgr") ||
            str_eq(*bts, "rgba") || str_eq(*bts, "bgra")))
        {
            return VException("Fourth argument must be 'rgb', 'bgr', 'rgba' or 'bgra'.");
        }
        
        if (str_eq(*bts, "rgb"))
            buf_type = BUF_RGB;
        else if (str_eq(*bts, "bgr"))
            buf_type = BUF_BGR;
        else if (str_eq(*bts, "rgba"))
            buf_type = BUF_RGBA;
        else if (str_eq(*bts, "bgra"))
            buf_type = BUF_BGRA;
        else
            return VException("Fourth argument wasn't 'rgb', 'bgr', 'rgba' or 'bgra'.");
    }

    Buffer *data = ObjectWrap::Unwrap<Buffer>(args[0]->ToObject());
    int w = args[1]->Int32Value();
    int h = args[2]->Int32Value();

    if (w < 0)
        return VException("Width smaller than 0.");
    if (h < 0)
        return VException("Height smaller than 0.");

    Gif *gif = new Gif(data, w, h, buf_type);
    gif->Wrap(args.This());
    return args.This();
}

Handle<Value>
Gif::GifEncode(const Arguments &args)
{
    HandleScope scope;

    Gif *gif = ObjectWrap::Unwrap<Gif>(args.This());
    return gif->GifEncode();
}

Handle<Value>
Gif::SetTransparencyColor(const Arguments &args)
{
    HandleScope scope;

    if (args.Length() != 3)
        return VException("Three arguments required - r, g, b");

    if (!args[0]->IsInt32())
        return VException("First argument must be integer red.");
    if (!args[1]->IsInt32())
        return VException("Second argument must be integer green.");
    if (!args[2]->IsInt32())
        return VException("Third argument must be integer blue.");
    
    unsigned char r = args[0]->Int32Value();
    unsigned char g = args[1]->Int32Value();
    unsigned char b = args[2]->Int32Value();

    Gif *gif = ObjectWrap::Unwrap<Gif>(args.This());
    gif->SetTransparencyColor(r, g, b);

    return Undefined();
}
