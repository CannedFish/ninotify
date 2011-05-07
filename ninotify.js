var ninotify = require('./src/build/default/ninotify.node'),
    fswapi = require('fswatch/lib/fswatchapi'),
    fsw2ni = {},
    ni2fsw = {};

fsw2ni[fswapi.CREATE] = ninotify.IN_CREATE | ninotify.IN_MOVED_TO;
fsw2ni[fswapi.DELETE] = ninotify.IN_DELETE | ninotify.IN_DELETE_SELF | ninotify.IN_MOVED_FROM;
fsw2ni[fswapi.MODIFY] = ninotify.IN_MODIFY;
fsw2ni[fswapi.STATS_CHANGED] = ninotify.IN_ATTR;
fsw2ni[fswapi.ALL] = ninotify.IN_CREATE | ninotify.IN_DELETE | ninotify.IN_DELETE_SELF |
              ninotify.IN_MODIFY | ninotify.IN_ATTR | ninotify.IN_MOVED_TO | ninotify.IN_MOVED_FROM;

ni2fsw[ninotify.IN_CREATE] = fswapi.CREATE;
ni2fsw[ninotify.IN_DELETE] = fswapi.DELETE;
ni2fsw[ninotify.IN_DELETE_SELF] = fswapi.DELETE;
ni2fsw[ninotify.IN_MODIFY] = fswapi.MODIFY;
ni2fsw[ninotify.IN_ATTR] = fswapi.STATS_CHANGED;
ni2fsw[ninotify.IN_MOVED_FROM] = fswapi.DELETE;
ni2fsw[ninotify.IN_MOVED_TO] = fswapi.CREATE;

exports.watcherid = 'ninotify';

exports.parserid = 'ninotify';

exports.parse = function(event) {
    var i = 0;

    return {
        type: event.substring(i, i = event.indexOf(' ', i)),
        mask: convertMask(new Number(event.substring(i + 1, i = event.indexOf(' ', i + 1))), ni2fsw),
        path: event.substring(i + 1, event.length)
    }
};

exports.addPath = function(path, mask, evpath) {
    ninotify.addWatch(path, convertMask(new Number(mask), fsw2ni), evpath);
};

function convertMask(mask, values) {
    var m = 0;

    for (var k in values) if (mask & k) m |= values[k];

    return m;
}
