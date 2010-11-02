var ninotify = require('./lib/build/default/ninotify.node');

exports.IN_ACCESS = ninotify.IN_ACCESS;
exports.IN_MODIFY = ninotify.IN_MODIFY;
exports.IN_ATTRIB = ninotify.IN_ATTRIB;
exports.IN_CLOSE_WRITE = ninotify.IN_CLOSE_WRITE;
exports.IN_CLOSE_NOWRITE = ninotify.IN_CLOSE_NOWRITE;
exports.IN_OPEN = ninotify.IN_OPEN;
exports.IN_MOVED_FROM = ninotify.IN_MOVED_FROM;
exports.IN_MOVED_TO = ninotify.IN_MOVED_TO;
exports.IN_CREATE = ninotify.IN_CREATE;
exports.IN_DELETE = ninotify.IN_DELETE;
exports.IN_DELETE_SELF = ninotify.IN_DELETE_SELF;

exports.addWatch = ninotify.addWatch;

exports.removeWatch = ninotify.removeWatch;

exports.removeAllWatches = ninotify.removeAllWatches;

