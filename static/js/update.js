function sleep(ms) {
    return new Promise(res => setTimeout(res, ms));
}

let Updater = function(update_fn, interval_ms) {
    let update_count = 0;

    async function update() {
        let cur_id = update_count + 1;
        ++update_count;

        while (update_count <= cur_id) {
            try {
                await update_fn();
            } catch (e) {
                console.error(e);
            } finally {
                await sleep(interval_ms);
            }
        }
    }

    return {
        update: update
    }
};
