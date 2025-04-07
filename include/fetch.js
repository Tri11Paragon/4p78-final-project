const v = await fetch('http://192.168.5.23/get_stuff', {
    'credentials': 'omit',
    'headers': {
        'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64; rv:136.0) Gecko/20100101 Firefox/136.0',
        'Accept': '/',
        'Accept-Language': 'en-US,en;q=0.5',
        'Priority': 'u=4'
    },
    'referrer': '$0',
    'method': 'GET',
    'mode': 'cors'
});
if (!v.ok) {
    return v.status;
}

const j = await v.json();
const floatArray = j.values;
Module.HEAPF32.set(floatArray, $1 >> 2);
