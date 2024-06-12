namespace Templ8
{
    /*
     * TargetsAttractSystem loops over each target component updating the steer component towards the target. It assumes
     * that each entity registered in targets is also registered in spatials and steers. It does not assume that each
     * entity registered in spatials and steers are also registered with targets.
     */
    void TargetsAttractSystem(vector<TargetComponent> targets, vector<SpatialComponent> spatials,
                              vector<SteerComponent> steers)
    {
        for (int i = 0; i < targets.size(); i++)
        {
            TargetComponent t = targets.at(i);
            SpatialComponent s = spatials.at(i);

            float2 toTarget = normalize( t.target - s.pos );
            float2 toRight = make_float2( -s.dir.y, s.dir.x );

            steers[i].steer = 2 * dot(toRight, toTarget);
        }
    }

    void MountainsRepelSystem(vector<SpatialComponent> spatials, vector<SteerComponent> steers, )
    {
        
    }

    void MovementSystem(vector<SpatialComponent> spatials, vector<MovementComponent> movements)
    {
        for (auto spatial : spatials)
        {
        }
    }
}
