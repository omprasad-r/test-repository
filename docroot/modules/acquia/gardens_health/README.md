Gardens Health
==============

Intro
-----
This module provides a simple interface for doing health checks on the gardens system. A health check can be any arbitrary code, but should use the GardensHealthReport class as a template. This will provide a common interface to retrieve health status messages.

Implementation
--------------
1. Extend GardensHealthReport to provide your health check function to the interface.
2. Implement hook_gardens_health_report(), which returns an array of GardensHealthReport class instances.
3. You can view the report at /admin/reports/gardens-health, but note that it is only visible to user 1.
