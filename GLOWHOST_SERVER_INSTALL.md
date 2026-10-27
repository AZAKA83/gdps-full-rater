# Glowhost custom-rater deployment request

The mod build is already on GitHub, but the GDPS needs a server-side add-on and one table. If your Glowhost dashboard does not expose server file upload and SQL import, send support this request:

> Please install the attached Cvolton/GMDprivateServer add-on files for my GDPS `https://glowhost.us/fpdexekj`:
> - `server-addon/cvolton/gdpsFullRaterGet.php` into the instance's `database/` directory
> - `server-addon/cvolton/gdpsFullRaterSet.php` into the same directory
> - import `server-addon/cvolton/schema.sql` into this GDPS's own database
>
> These endpoints store a separate custom difficulty tier per level and use the existing Cvolton GJP2 authentication plus `commandRate` permission. Please confirm whether custom PHP/API endpoints and a custom table are allowed for this hosted project.

Do not send passwords, database passwords, GJP2 tokens, or OAuth codes. Glowhost contact: https://www.glowhost.us/contact
