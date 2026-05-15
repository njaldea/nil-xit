from __future__ import annotations

import ctypes
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Callable, Dict, List, Optional
from inspect import signature

import nil_service


# ---------------------------------------------------------------------------
# C structure wrappers
# ---------------------------------------------------------------------------


class NilXitCore(ctypes.Structure):
    _fields_ = [("handle", ctypes.c_void_p)]


class NilXitUniqueFrame(ctypes.Structure):
    _fields_ = [("handle", ctypes.c_void_p)]


class NilXitUniqueFrameValue(ctypes.Structure):
    _fields_ = [("handle", ctypes.c_void_p)]


class NilXitTaggedFrame(ctypes.Structure):
    _fields_ = [("handle", ctypes.c_void_p)]


class NilXitTaggedFrameValue(ctypes.Structure):
    _fields_ = [("handle", ctypes.c_void_p)]


class NilXitGroupEntry(ctypes.Structure):
    _fields_ = [
        ("group", ctypes.c_char_p),
        ("path", ctypes.c_char_p),
    ]


class NilXitFileInfo(ctypes.Structure):
    _fields_ = [
        ("group", ctypes.c_char_p),
        ("path", ctypes.c_char_p),
    ]


@dataclass
class FileInfo:
    group: str
    path: str


# ---------------------------------------------------------------------------
# Callback function pointer types
# ---------------------------------------------------------------------------

NIL_XIT_UNIQUE_EXEC = ctypes.CFUNCTYPE(None, ctypes.c_void_p)
NIL_XIT_UNIQUE_SIGNAL_EXEC = ctypes.CFUNCTYPE(
    None, ctypes.c_void_p, ctypes.c_uint64, ctypes.c_void_p
)
NIL_XIT_UNIQUE_ON_SUB_EXEC = ctypes.CFUNCTYPE(None, ctypes.c_uint64, ctypes.c_void_p)
NIL_XIT_TAGGED_EXEC = ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_void_p)
NIL_XIT_TAGGED_SIGNAL_EXEC = ctypes.CFUNCTYPE(
    None, ctypes.c_char_p, ctypes.c_void_p, ctypes.c_uint64, ctypes.c_void_p
)
NIL_XIT_TAGGED_ON_SUB_EXEC = ctypes.CFUNCTYPE(
    None, ctypes.c_char_p, ctypes.c_uint64, ctypes.c_void_p
)
NIL_XIT_CLEANUP = ctypes.CFUNCTYPE(None, ctypes.c_void_p)

# encode_size(ctx) -> uint64
NIL_XIT_UNIQUE_ENCODE_SIZE = ctypes.CFUNCTYPE(ctypes.c_uint64, ctypes.c_void_p)
# encode(ctx, buffer)
NIL_XIT_UNIQUE_ENCODE = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_void_p)
# decode(ctx, buffer, size)
NIL_XIT_UNIQUE_DECODE = ctypes.CFUNCTYPE(
    None, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_uint64
)

# encode_size(tag, ctx) -> uint64
NIL_XIT_TAGGED_ENCODE_SIZE = ctypes.CFUNCTYPE(
    ctypes.c_uint64, ctypes.c_char_p, ctypes.c_void_p
)
# encode(tag, ctx, buffer)
NIL_XIT_TAGGED_ENCODE = ctypes.CFUNCTYPE(
    None, ctypes.c_char_p, ctypes.c_void_p, ctypes.c_void_p
)
# decode(tag, ctx, buffer, size)
NIL_XIT_TAGGED_DECODE = ctypes.CFUNCTYPE(
    None, ctypes.c_char_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_uint64
)


# ---------------------------------------------------------------------------
# Info structures
# ---------------------------------------------------------------------------


class NilXitUniqueCallbackInfo(ctypes.Structure):
    _fields_ = [
        ("exec", NIL_XIT_UNIQUE_EXEC),
        ("context", ctypes.c_void_p),
        ("cleanup", NIL_XIT_CLEANUP),
    ]


class NilXitUniqueSignalCallbackInfo(ctypes.Structure):
    _fields_ = [
        ("exec", NIL_XIT_UNIQUE_SIGNAL_EXEC),
        ("context", ctypes.c_void_p),
        ("cleanup", NIL_XIT_CLEANUP),
    ]


class NilXitUniqueOnSubInfo(ctypes.Structure):
    _fields_ = [
        ("exec", NIL_XIT_UNIQUE_ON_SUB_EXEC),
        ("context", ctypes.c_void_p),
        ("cleanup", NIL_XIT_CLEANUP),
    ]


class NilXitTaggedCallbackInfo(ctypes.Structure):
    _fields_ = [
        ("exec", NIL_XIT_TAGGED_EXEC),
        ("context", ctypes.c_void_p),
        ("cleanup", NIL_XIT_CLEANUP),
    ]


class NilXitTaggedSignalCallbackInfo(ctypes.Structure):
    _fields_ = [
        ("exec", NIL_XIT_TAGGED_SIGNAL_EXEC),
        ("context", ctypes.c_void_p),
        ("cleanup", NIL_XIT_CLEANUP),
    ]


class NilXitTaggedOnSubInfo(ctypes.Structure):
    _fields_ = [
        ("exec", NIL_XIT_TAGGED_ON_SUB_EXEC),
        ("context", ctypes.c_void_p),
        ("cleanup", NIL_XIT_CLEANUP),
    ]


class NilXitUniqueValueAccessor(ctypes.Structure):
    _fields_ = [
        ("encode_size", NIL_XIT_UNIQUE_ENCODE_SIZE),
        ("encode", NIL_XIT_UNIQUE_ENCODE),
        ("decode", NIL_XIT_UNIQUE_DECODE),
        ("ctx", ctypes.c_void_p),
        ("cleanup", NIL_XIT_CLEANUP),
    ]


class NilXitTaggedValueAccessor(ctypes.Structure):
    _fields_ = [
        ("encode_size", NIL_XIT_TAGGED_ENCODE_SIZE),
        ("encode", NIL_XIT_TAGGED_ENCODE),
        ("decode", NIL_XIT_TAGGED_DECODE),
        ("ctx", ctypes.c_void_p),
        ("cleanup", NIL_XIT_CLEANUP),
    ]


# ---------------------------------------------------------------------------
# Internal state helpers
# ---------------------------------------------------------------------------


def _to_ref_id(ptr: Any) -> int:
    if ptr is None:
        return 0
    if isinstance(ptr, int):
        return ptr
    return int(ctypes.cast(ptr, ctypes.c_void_p).value or 0)


@dataclass
class _CallbackState:
    fn: Callable[..., Any]


@dataclass
class _AccessorState:
    encode_fn: Callable[..., Any]
    decode_fn: Callable[..., Any]
    _cached: Optional[bytes] = field(default=None)


# ---------------------------------------------------------------------------
# Library signature configuration
# ---------------------------------------------------------------------------


def _configure_signatures(lib: Any) -> None:
    lib.nil_xit_core_create.argtypes = [nil_service.NilServiceRunnable, nil_service.NilServiceEvent]
    lib.nil_xit_core_create.restype = NilXitCore

    lib.nil_xit_core_create_from_standalone.argtypes = [nil_service.NilServiceStandalone]
    lib.nil_xit_core_create_from_standalone.restype = NilXitCore

    lib.nil_xit_setup_server.argtypes = [
        nil_service.NilServiceWeb,
        ctypes.POINTER(ctypes.c_char_p),
        ctypes.c_size_t,
    ]
    lib.nil_xit_setup_server.restype = None

    lib.nil_xit_set_cache_directory.argtypes = [NilXitCore, ctypes.c_char_p]
    lib.nil_xit_set_cache_directory.restype = None

    lib.nil_xit_set_groups.argtypes = [
        NilXitCore,
        ctypes.POINTER(NilXitGroupEntry),
        ctypes.c_uint64,
    ]
    lib.nil_xit_set_groups.restype = None

    lib.nil_xit_core_destroy.argtypes = [NilXitCore]
    lib.nil_xit_core_destroy.restype = None

    lib.nil_xit_core_add_unique_frame.argtypes = [
        NilXitCore,
        ctypes.c_char_p,
        ctypes.POINTER(NilXitFileInfo),
    ]
    lib.nil_xit_core_add_unique_frame.restype = NilXitUniqueFrame

    lib.nil_xit_core_add_tagged_frame.argtypes = [
        NilXitCore,
        ctypes.c_char_p,
        ctypes.POINTER(NilXitFileInfo),
    ]
    lib.nil_xit_core_add_tagged_frame.restype = NilXitTaggedFrame

    lib.nil_xit_unique_frame_on_load.argtypes = [
        NilXitUniqueFrame,
        NilXitUniqueCallbackInfo,
    ]
    lib.nil_xit_unique_frame_on_load.restype = None

    lib.nil_xit_unique_frame_on_sub.argtypes = [
        NilXitUniqueFrame,
        NilXitUniqueOnSubInfo,
    ]
    lib.nil_xit_unique_frame_on_sub.restype = None

    lib.nil_xit_tagged_frame_on_load.argtypes = [
        NilXitTaggedFrame,
        NilXitTaggedCallbackInfo,
    ]
    lib.nil_xit_tagged_frame_on_load.restype = None

    lib.nil_xit_tagged_frame_on_sub.argtypes = [
        NilXitTaggedFrame,
        NilXitTaggedOnSubInfo,
    ]
    lib.nil_xit_tagged_frame_on_sub.restype = None

    lib.nil_xit_unique_frame_add_value.argtypes = [
        NilXitUniqueFrame,
        ctypes.c_char_p,
        NilXitUniqueValueAccessor,
    ]
    lib.nil_xit_unique_frame_add_value.restype = NilXitUniqueFrameValue

    lib.nil_xit_tagged_frame_add_value.argtypes = [
        NilXitTaggedFrame,
        ctypes.c_char_p,
        NilXitTaggedValueAccessor,
    ]
    lib.nil_xit_tagged_frame_add_value.restype = NilXitTaggedFrameValue

    lib.nil_xit_unique_frame_add_option.argtypes = [
        NilXitUniqueFrame,
        ctypes.c_char_p,
        ctypes.c_char_p,
    ]
    lib.nil_xit_unique_frame_add_option.restype = None

    lib.nil_xit_tagged_frame_add_option.argtypes = [
        NilXitTaggedFrame,
        ctypes.c_char_p,
        ctypes.c_char_p,
    ]
    lib.nil_xit_tagged_frame_add_option.restype = None

    lib.nil_xit_unique_frame_add_signal.argtypes = [
        NilXitUniqueFrame,
        ctypes.c_char_p,
        NilXitUniqueSignalCallbackInfo,
    ]
    lib.nil_xit_unique_frame_add_signal.restype = None

    lib.nil_xit_tagged_frame_add_signal.argtypes = [
        NilXitTaggedFrame,
        ctypes.c_char_p,
        NilXitTaggedSignalCallbackInfo,
    ]
    lib.nil_xit_tagged_frame_add_signal.restype = None

    lib.nil_xit_unique_value_post.argtypes = [
        NilXitUniqueFrameValue,
        ctypes.c_void_p,
        ctypes.c_uint64,
    ]
    lib.nil_xit_unique_value_post.restype = None

    lib.nil_xit_tagged_value_post.argtypes = [
        NilXitTaggedFrameValue,
        ctypes.c_char_p,
        ctypes.c_void_p,
        ctypes.c_uint64,
    ]
    lib.nil_xit_tagged_value_post.restype = None


# ---------------------------------------------------------------------------
# Callback function implementations
# ---------------------------------------------------------------------------


def _create_lib_fns(refs: Dict[int, Any], lib: Any) -> dict:
    libc = ctypes.CDLL(None)
    libc.malloc.argtypes = [ctypes.c_size_t]
    libc.malloc.restype = ctypes.c_void_p
    libc.free.argtypes = [ctypes.c_void_p]
    libc.free.restype = None

    @NIL_XIT_UNIQUE_EXEC
    def unique_exec(ctx_id: Any) -> None:
        ref_id = _to_ref_id(ctx_id)
        if ref_id not in refs:
            return
        refs[ref_id].fn()

    @NIL_XIT_UNIQUE_SIGNAL_EXEC
    def unique_signal_exec(data: Any, size: int, ctx_id: Any) -> None:
        ref_id = _to_ref_id(ctx_id)
        if ref_id not in refs:
            return
        payload = b""
        if size:
            payload = ctypes.string_at(data, int(size))
        refs[ref_id].fn(payload)

    @NIL_XIT_UNIQUE_ON_SUB_EXEC
    def unique_on_sub_exec(count: int, ctx_id: Any) -> None:
        ref_id = _to_ref_id(ctx_id)
        if ref_id not in refs:
            return
        refs[ref_id].fn(int(count))

    @NIL_XIT_TAGGED_EXEC
    def tagged_exec(tag: Any, ctx_id: Any) -> None:
        ref_id = _to_ref_id(ctx_id)
        if ref_id not in refs:
            return
        refs[ref_id].fn(tag.decode("utf-8") if tag else "")

    @NIL_XIT_TAGGED_SIGNAL_EXEC
    def tagged_signal_exec(tag: Any, data: Any, size: int, ctx_id: Any) -> None:
        ref_id = _to_ref_id(ctx_id)
        if ref_id not in refs:
            return
        tag_value = tag.decode("utf-8") if tag else ""
        payload = b""
        if size:
            payload = ctypes.string_at(data, int(size))
        refs[ref_id].fn(tag_value, payload)

    @NIL_XIT_TAGGED_ON_SUB_EXEC
    def tagged_on_sub_exec(tag: Any, count: int, ctx_id: Any) -> None:
        ref_id = _to_ref_id(ctx_id)
        if ref_id not in refs:
            return
        refs[ref_id].fn(tag.decode("utf-8") if tag else "", int(count))

    @NIL_XIT_UNIQUE_ENCODE_SIZE
    def unique_encode_size_exec(ctx_id: Any) -> int:
        ref_id = _to_ref_id(ctx_id)
        if ref_id not in refs:
            return 0
        entry = refs[ref_id]
        entry._cached = entry.encode_fn()
        return len(entry._cached) if entry._cached else 0

    @NIL_XIT_UNIQUE_ENCODE
    def unique_encode_exec(ctx_id: Any, buffer: Any) -> None:
        ref_id = _to_ref_id(ctx_id)
        if ref_id not in refs:
            return
        entry = refs[ref_id]
        if entry._cached:
            ctypes.memmove(buffer, entry._cached, len(entry._cached))
            entry._cached = None

    @NIL_XIT_UNIQUE_DECODE
    def unique_decode_exec(ctx_id: Any, buffer: Any, size: int) -> None:
        ref_id = _to_ref_id(ctx_id)
        if ref_id not in refs:
            return
        entry = refs[ref_id]
        entry.decode_fn(ctypes.string_at(buffer, int(size)))

    @NIL_XIT_TAGGED_ENCODE_SIZE
    def tagged_encode_size_exec(tag: Any, ctx_id: Any) -> int:
        ref_id = _to_ref_id(ctx_id)
        if ref_id not in refs:
            return 0
        entry = refs[ref_id]
        entry._cached = entry.encode_fn(tag.decode("utf-8") if tag else "")
        return len(entry._cached) if entry._cached else 0

    @NIL_XIT_TAGGED_ENCODE
    def tagged_encode_exec(tag: Any, ctx_id: Any, buffer: Any) -> None:
        ref_id = _to_ref_id(ctx_id)
        if ref_id not in refs:
            return
        entry = refs[ref_id]
        if entry._cached:
            ctypes.memmove(buffer, entry._cached, len(entry._cached))
            entry._cached = None

    @NIL_XIT_TAGGED_DECODE
    def tagged_decode_exec(tag: Any, ctx_id: Any, buffer: Any, size: int) -> None:
        ref_id = _to_ref_id(ctx_id)
        if ref_id not in refs:
            return
        entry = refs[ref_id]
        entry.decode_fn(
            tag.decode("utf-8") if tag else "",
            ctypes.string_at(buffer, int(size)),
        )

    @NIL_XIT_CLEANUP
    def cleanup(ctx_id: Any) -> None:
        if ctx_id:
            ref_id = _to_ref_id(ctx_id)
            refs.pop(ref_id, None)
            libc.free(ctx_id)

    def store_callback(fn: Callable[..., Any]) -> Any:
        ctx_id = libc.malloc(1)
        if not ctx_id:
            raise MemoryError("malloc failed")
        refs[_to_ref_id(ctx_id)] = _CallbackState(fn=fn)
        return ctx_id

    def store_accessor(
        encode_fn: Callable[..., Any], decode_fn: Callable[..., Any]
    ) -> Any:
        ctx_id = libc.malloc(1)
        if not ctx_id:
            raise MemoryError("malloc failed")
        refs[_to_ref_id(ctx_id)] = _AccessorState(
            encode_fn=encode_fn, decode_fn=decode_fn
        )
        return ctx_id

    return {
        "unique_exec": unique_exec,
        "unique_signal_exec": unique_signal_exec,
        "unique_on_sub_exec": unique_on_sub_exec,
        "tagged_exec": tagged_exec,
        "tagged_signal_exec": tagged_signal_exec,
        "tagged_on_sub_exec": tagged_on_sub_exec,
        "unique_encode_size_exec": unique_encode_size_exec,
        "unique_encode_exec": unique_encode_exec,
        "unique_decode_exec": unique_decode_exec,
        "tagged_encode_size_exec": tagged_encode_size_exec,
        "tagged_encode_exec": tagged_encode_exec,
        "tagged_decode_exec": tagged_decode_exec,
        "cleanup": cleanup,
        "store_callback": store_callback,
        "store_accessor": store_accessor,
    }


# ---------------------------------------------------------------------------
# Public API wrappers
# ---------------------------------------------------------------------------


class UniqueValue:
    def __init__(self, value: NilXitUniqueFrameValue, lib: Any) -> None:
        self._value = value
        self._lib = lib

    def post(self, data: bytes) -> None:
        self._lib.nil_xit_unique_value_post(self._value, data, len(data))


class TaggedValue:
    def __init__(self, value: NilXitTaggedFrameValue, lib: Any) -> None:
        self._value = value
        self._lib = lib

    def post(self, tag: str, data: bytes) -> None:
        self._lib.nil_xit_tagged_value_post(
            self._value, tag.encode("utf-8"), data, len(data)
        )


class UniqueFrame:
    def __init__(self, frame: NilXitUniqueFrame, lib: Any, fns: dict) -> None:
        self._frame = frame
        self._lib = lib
        self._fns = fns

    def on_load(self, fn: Callable[[], None]) -> None:
        info = NilXitUniqueCallbackInfo(
            exec=self._fns["unique_exec"],
            context=self._fns["store_callback"](fn),
            cleanup=self._fns["cleanup"],
        )
        self._lib.nil_xit_unique_frame_on_load(self._frame, info)

    def on_sub(self, fn: Callable[[int], None]) -> None:
        info = NilXitUniqueOnSubInfo(
            exec=self._fns["unique_on_sub_exec"],
            context=self._fns["store_callback"](fn),
            cleanup=self._fns["cleanup"],
        )
        self._lib.nil_xit_unique_frame_on_sub(self._frame, info)

    def add_value(
        self,
        id: str,
        encode_fn: Callable[[], bytes],
        decode_fn: Callable[[bytes], None],
    ) -> UniqueValue:
        acc = NilXitUniqueValueAccessor(
            encode_size=self._fns["unique_encode_size_exec"],
            encode=self._fns["unique_encode_exec"],
            decode=self._fns["unique_decode_exec"],
            ctx=self._fns["store_accessor"](encode_fn, decode_fn),
            cleanup=self._fns["cleanup"],
        )
        value = self._lib.nil_xit_unique_frame_add_value(
            self._frame, id.encode("utf-8"), acc
        )
        return UniqueValue(value, self._lib)

    def add_signal(self, id: str, fn: Callable[[bytes], None]) -> None:
        last_fn = fn
        try:
            param_count = len(signature(fn).parameters)
        except (TypeError, ValueError):
            param_count = 1
        if param_count == 0:
            def wfn(_: bytes) -> None:
                fn()
            last_fn = wfn
        info = NilXitUniqueSignalCallbackInfo(
            exec=self._fns["unique_signal_exec"],
            context=self._fns["store_callback"](last_fn),
            cleanup=self._fns["cleanup"],
        )
        self._lib.nil_xit_unique_frame_add_signal(
            self._frame, id.encode("utf-8"), info
        )

    def add_option(self, key: str, value: str) -> None:
        self._lib.nil_xit_unique_frame_add_option(
            self._frame,
            key.encode("utf-8"),
            value.encode("utf-8"),
        )


class TaggedFrame:
    def __init__(self, frame: NilXitTaggedFrame, lib: Any, fns: dict) -> None:
        self._frame = frame
        self._lib = lib
        self._fns = fns

    def on_load(self, fn: Callable[[str], None]) -> None:
        info = NilXitTaggedCallbackInfo(
            exec=self._fns["tagged_exec"],
            context=self._fns["store_callback"](fn),
            cleanup=self._fns["cleanup"],
        )
        self._lib.nil_xit_tagged_frame_on_load(self._frame, info)

    def on_sub(self, fn: Callable[[str, int], None]) -> None:
        info = NilXitTaggedOnSubInfo(
            exec=self._fns["tagged_on_sub_exec"],
            context=self._fns["store_callback"](fn),
            cleanup=self._fns["cleanup"],
        )
        self._lib.nil_xit_tagged_frame_on_sub(self._frame, info)

    def add_value(
        self,
        id: str,
        encode_fn: Callable[[str], bytes],
        decode_fn: Callable[[str, bytes], None],
    ) -> TaggedValue:
        acc = NilXitTaggedValueAccessor(
            encode_size=self._fns["tagged_encode_size_exec"],
            encode=self._fns["tagged_encode_exec"],
            decode=self._fns["tagged_decode_exec"],
            ctx=self._fns["store_accessor"](encode_fn, decode_fn),
            cleanup=self._fns["cleanup"],
        )
        value = self._lib.nil_xit_tagged_frame_add_value(
            self._frame, id.encode("utf-8"), acc
        )
        return TaggedValue(value, self._lib)

    def add_signal(self, id: str, fn: Callable[[str, bytes], None]) -> None:
        last_fn = fn
        try:
            param_count = len(signature(fn).parameters)
        except (TypeError, ValueError):
            param_count = 2
        if param_count == 1:
            def wfn(tag: str, _: bytes) -> None:
                fn(tag)
            last_fn = wfn
        info = NilXitTaggedSignalCallbackInfo(
            exec=self._fns["tagged_signal_exec"],
            context=self._fns["store_callback"](last_fn),
            cleanup=self._fns["cleanup"],
        )
        self._lib.nil_xit_tagged_frame_add_signal(
            self._frame, id.encode("utf-8"), info
        )

    def add_option(self, key: str, value: str) -> None:
        self._lib.nil_xit_tagged_frame_add_option(
            self._frame,
            key.encode("utf-8"),
            value.encode("utf-8"),
        )


class Core:
    def __init__(self, core: NilXitCore, lib: Any, fns: dict) -> None:
        self._core = core
        self._lib = lib
        self._fns = fns

    def set_cache_directory(self, path: Optional[str]) -> None:
        if path is None:
            self._lib.nil_xit_set_cache_directory(self._core, None)
            return
        self._lib.nil_xit_set_cache_directory(self._core, path.encode("utf-8"))

    def set_groups(self, groups: Dict[str, str]) -> None:
        count = len(groups)
        entries = (NilXitGroupEntry * count)()
        for i, (group, path) in enumerate(groups.items()):
            entries[i].group = group.encode("utf-8")
            entries[i].path = path.encode("utf-8")
        self._lib.nil_xit_set_groups(self._core, entries, count)

    def add_unique_frame(self, id: str, info: Optional[FileInfo] = None) -> UniqueFrame:
        file_info = None
        if info is not None:
            file_info = NilXitFileInfo(
                group=info.group.encode("utf-8"),
                path=info.path.encode("utf-8"),
            )
        frame = self._lib.nil_xit_core_add_unique_frame(
            self._core,
            id.encode("utf-8"),
            ctypes.byref(file_info) if file_info is not None else None,
        )
        return UniqueFrame(frame, self._lib, self._fns)

    def add_tagged_frame(self, id: str, info: Optional[FileInfo] = None) -> TaggedFrame:
        file_info = None
        if info is not None:
            file_info = NilXitFileInfo(
                group=info.group.encode("utf-8"),
                path=info.path.encode("utf-8"),
            )
        frame = self._lib.nil_xit_core_add_tagged_frame(
            self._core,
            id.encode("utf-8"),
            ctypes.byref(file_info) if file_info is not None else None,
        )
        return TaggedFrame(frame, self._lib, self._fns)

    def destroy(self) -> None:
        self._lib.nil_xit_core_destroy(self._core)


# ---------------------------------------------------------------------------
# Module
# ---------------------------------------------------------------------------


class Module:
    def __init__(self) -> None:
        lib_path = Path(__file__).resolve().parent / "libnil-xit-c-api.so"
        self._lib = ctypes.CDLL(str(lib_path))
        _configure_signatures(self._lib)
        self._refs: Dict[int, Any] = {}
        self._fns = _create_lib_fns(self._refs, self._lib)

    def create_core(self, runnable: nil_service.Runnable, event: nil_service.Event) -> Core:
        core = self._lib.nil_xit_core_create(runnable._runnable, event._event)
        return Core(core, self._lib, self._fns)

    def create_core_from_standalone(self, standalone: nil_service.Standalone) -> Core:
        core = self._lib.nil_xit_core_create_from_standalone(standalone._standalone)
        return Core(core, self._lib, self._fns)

    def setup_server(self, web: nil_service.Web, paths: Optional[List[str]] = None) -> None:
        local_paths = list(paths or [])
        assets_dir = str((Path(__file__).resolve().parent / "assets"))
        if assets_dir not in local_paths:
            local_paths.append(assets_dir)
        count = len(local_paths)
        arr = (ctypes.c_char_p * count)(*[p.encode("utf-8") for p in local_paths])
        self._lib.nil_xit_setup_server(web._web, arr, count)


_XIT = Module()


def create_core(runnable: nil_service.Runnable, event: nil_service.Event) -> Core:
    return _XIT.create_core(runnable, event)


def create_core_from_standalone(standalone: nil_service.Standalone) -> Core:
    return _XIT.create_core_from_standalone(standalone)


def setup_server(web: nil_service.Web, paths: Optional[List[str]] = None) -> None:
    _XIT.setup_server(web, paths)


__all__ = [
    "create_core",
    "create_core_from_standalone",
    "setup_server",
    "Core",
    "FileInfo",
    "UniqueFrame",
    "TaggedFrame",
    "UniqueValue",
    "TaggedValue",
]
