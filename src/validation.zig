// Zig entrypoint for the validation logic of the PHP extension

const std = @import("std");
const c = @cImport({
    @cInclude("php.h");
    @cInclude("zend_API.h");
});

pub export fn zig_validate(data: *c.zval, model: *c.zval) callconv(.C) c_int {
    std.log.debug("zig_validate", .{});
    return 0;
}

pub export fn zig_validate_callable(data: *c.zval, call: *c.zval) callconv(.C) c_int {
    std.log.debug("zig_validate_callable", .{});
    return 0;
}
